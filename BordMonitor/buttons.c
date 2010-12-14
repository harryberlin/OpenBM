#include "buttons.h"
#include "i2cmaster.h"
#include "base.h"
#include "leds.h"

#define ENABLE_RADIO_BUTTON_LEFT_PART() {PORTC &= ~(1 << 6); PORTC |= (1 << 7);}
#define ENABLE_RADIO_BUTTON_RIGHT_PART() {PORTC &= ~(1 << 7); PORTC |= (1 << 6);}
#define ENABLE_RADIO_BUTTON_BOTH_PARTS() {PORTC &= ~(1 << 7); PORTC &= ~(1 << 6);}

buttonGlobalState_t g_buttons = 0;
buttonGlobalState_t g_buttonsDown = 0;
buttonGlobalState_t g_buttonsPressed = 0;
buttonGlobalState_t g_buttonsRelease = 0;
buttonGlobalState_t g_buttons_last = 0;

uint8_t g_buttons_ExpanderState = 0;

// delay in ticks time steps for each button
uint8_t g_button_delay[BUTTON_NUM_BUTTONS];

// direction states of both encoders
volatile uint8_t g_encoder_flag = 0;
volatile int8_t g_enc_delta[2];
int8_t g_enc_last_radioRotaryStateCW;
int8_t g_enc_last_radioRotaryStateCCW;
int8_t g_enc_last[2];
#define ENC_BMBT_KNOB (1 << 0)
#define ENC_BMBT_OUT1 (1 << 1)
#define ENC_BMBT_OUT2 (1 << 2)
#define BMBT_EJECT    (1 << 3)
#define ENC_BMBT 0
#define ENC_RADIO_OUT2 (1 << 0)
#define ENC_RADIO_OUT1 (1 << 1)
#define ENC_RADIO_KNOB (1 << 2)
#define ENC_RADIO 1

//------------------------------------------------------------------------------
buttonBool_t button(buttonIndex_t id) { return is_bit_set(g_buttons,id); }
buttonBool_t button_down(buttonIndex_t id) { return is_bit_set(g_buttonsDown,id); }
buttonBool_t button_pressed(buttonIndex_t id) { return is_bit_set(g_buttonsPressed,id); }
buttonBool_t button_released(buttonIndex_t id) { return is_bit_set(g_buttonsRelease,id); }

//------------------------------------------------------------------------------
buttonGlobalState_t button_global(void) { return g_buttons; }
buttonGlobalState_t button_global_down(void) { return g_buttonsDown; }
buttonGlobalState_t button_global_pressed(void) { return g_buttonsPressed; }
buttonGlobalState_t button_global_released(void) { return g_buttonsRelease; }

//------------------------------------------------------------------------------
uint8_t button_down_time(buttonIndex_t id)
{
    return g_button_delay[id];
}

//------------------------------------------------------------------------------
void button_init(void)
{
    // setup INT1 pin as input
    DDRD &= ~(1 << DDD3);
    PORTD |= (1 << 3);

    // set bmbt encoder expander to high all pins
    if (i2c_start_wait(PORT_EXPANDER_ENC_BMBT + I2C_WRITE, 100) == 0)
    {
        i2c_write(0xFF);
        i2c_stop();
    }

    // setup radio encoder port expander into read mode
    if (i2c_start_wait(PORT_EXPANDER_ENC_RADIO + I2C_WRITE, 100) == 0)
    {
        i2c_write(3);       // write to configuration register
        i2c_write(0xFF);    // set all pins as input
        i2c_stop();
    }
    if (i2c_start_wait(PORT_EXPANDER_ENC_RADIO + I2C_WRITE, 100) == 0)
    {
        i2c_write(2);       // write to polarity inversion register
        i2c_write(0xFF);    // set all inputs to return inverted polarity
        i2c_stop();
    }

    // get current state of encoders so that we have a start value for them
    if (i2c_start(PORT_EXPANDER_ENC_BMBT + I2C_READ) == 0)
    {
        unsigned char state = ~(i2c_readNak());
        i2c_stop();

        // power-on state of bmbt encoder
        int8_t new = 0;
        if( state & ENC_BMBT_OUT1 ) new  = 0b11;
        if( state & ENC_BMBT_OUT2 ) new ^= 0b01;
        g_enc_last[ENC_BMBT] = new;
    }
    g_enc_delta[ENC_BMBT] = 0;

    // read from radio encoders, has special treatment (need to send read command first)
    if (i2c_start(PORT_EXPANDER_ENC_RADIO + I2C_WRITE) == 0)
    {
        i2c_write(0);   // set into read mode
        if (i2c_rep_start(PORT_EXPANDER_ENC_RADIO + I2C_READ) == 0)
        {
            unsigned char state = i2c_readNak();

            // power-on state of radio encoder
            int8_t new = 0;
            if( state & ENC_RADIO_OUT1 ) new  = 0b11;
            if( state & ENC_RADIO_OUT2 ) new ^= 0b01;
            g_enc_last[ENC_RADIO] = new;

            g_enc_last_radioRotaryStateCW = 0;
            g_enc_last_radioRotaryStateCCW = 0;
        }
        i2c_stop();
    }
    g_enc_delta[ENC_RADIO] = 0;


    for (int i=0; i < BUTTON_NUM_BUTTONS; i++) g_button_delay[i] = 0;
    
    g_buttons = 0;
    g_buttonsDown = 0;
    g_buttonsPressed = 0;
    g_buttonsRelease = 0;

    g_buttons_last = 0;

    g_encoder_flag = 0;

    // button switcher on radio PCB
    DDRC |= (1 << 6) | (1 << 7);
    ENABLE_RADIO_BUTTON_BOTH_PARTS();
    //ENABLE_RADIO_BUTTON_RIGHT_PART();
    
    // interrupt for the rotary encoders' port expander
    EIMSK &= ~(1 << INT1);                              // disable interrupt
    EICRA = (EICRA | (1 << ISC11)) & (~(1 << ISC10));   // enable falling edge
    EIMSK |= (1 << INT1);                               // enable interrupt
}

//------------------------------------------------------------------------------
void button_tick_encoder(void)
{
    // clear pressed/release states
    g_buttonsPressed = 0;
    g_buttonsRelease = 0;
    
    // clear rotary states of both encoders, this will make sure
    // that if flag get active, this will happen only for the duration until next
    // call of this routine
    g_buttons &= ~(BUTTON_ROTARY_STATE_MASK);

    // if no event happened, then don't do anything
    if (g_encoder_flag == 0) return;
    g_encoder_flag = 0;

    // ----- Get value for RADIO encoder -----
    if (i2c_start(PORT_EXPANDER_ENC_RADIO + I2C_WRITE) == 0)
    {
        i2c_write(0);   // set into read mode
        if (i2c_rep_start(PORT_EXPANDER_ENC_RADIO + I2C_READ) == 0)
        {
            unsigned char state = i2c_readNak();
            i2c_stop();

            if (state & ENC_RADIO_KNOB) g_buttons |= (1L << BUTTON_RADIO_KNOB);
            else g_buttons &= ~(1L << BUTTON_RADIO_KNOB);

            int8_t new, diff;

            // get value for radio encoder
            new = 0;
            if( (state & ENC_RADIO_OUT1) ) new  = 0b11;
            if( (state & ENC_RADIO_OUT2) ) new ^= 0b01;

            diff = g_enc_last[ENC_RADIO] - new;
            if( diff & 1 )
            {
                g_enc_last[ENC_RADIO] = new;
                g_enc_delta[ENC_RADIO] += (diff & 2) - 1;		// bit 1 = direction (+/-)
            }
            if (g_enc_delta[ENC_RADIO] < 0 && !g_enc_last_radioRotaryStateCCW)
            {
                g_buttons |= (1L << BUTTON_RADIO_CCW);
                g_buttons &= ~(1L << BUTTON_RADIO_CW);
            }else if (g_enc_delta[ENC_RADIO] > 0 && !g_enc_last_radioRotaryStateCW)
            {
                g_buttons |= (1L << BUTTON_RADIO_CW);
                g_buttons &= ~(1L << BUTTON_RADIO_CCW);
            }
            g_enc_delta[ENC_RADIO] = 0;
        }else
            i2c_stop();
    }

    g_enc_last_radioRotaryStateCCW = button(BUTTON_RADIO_CCW);
    g_enc_last_radioRotaryStateCW  = button(BUTTON_RADIO_CW);

    // interrupt was cleared, so do not need to check for other stuff
    if (bit_is_set(PIND,3)) return;
    
    // ----- Get value for BMBT encoder -----
    if (i2c_start(PORT_EXPANDER_ENC_BMBT + I2C_READ) == 0)
    {
        unsigned char state = ~(i2c_readNak());
        i2c_stop();

        // get current button states
        if (state & ENC_BMBT_KNOB) g_buttons |= (1L << BUTTON_BMBT_KNOB);
        else g_buttons &= ~(1L << BUTTON_BMBT_KNOB);
        if (state & BMBT_EJECT) g_buttons |= (1L << BUTTON_EJECT);
        else g_buttons &= ~(1L << BUTTON_EJECT);
        
        int8_t new, diff;
        
        // get value for bmbt encoder
        new = 0;
        if( (state & ENC_BMBT_OUT1) ) new  = 0b11;
        if( (state & ENC_BMBT_OUT2) ) new ^= 0b01;
        diff = g_enc_last[ENC_BMBT] - new;

        if( diff & 1 )
        {
            g_enc_last[ENC_BMBT] = new;
            g_enc_delta[ENC_BMBT] += (diff & 2) - 1;		// bit 1 = direction (+/-)
        }
        if (g_enc_delta[ENC_BMBT] < 0)
        {
            g_buttons |= (1L << BUTTON_BMBT_CCW);
            g_buttons &= ~(1L << BUTTON_BMBT_CW);
        }else if (g_enc_delta[ENC_BMBT] > 0)
        {
            g_buttons |= (1L << BUTTON_BMBT_CW);
            g_buttons &= ~(1L << BUTTON_BMBT_CCW);
        }
        g_enc_delta[ENC_BMBT] = 0;
    }

}

//------------------------------------------------------------------------------
void button_tick(void)
{
    // if no active interrupt, then we do not have to test for buttons
    if (bit_is_set(PIND,3)) goto nextkeys;

    if (bit_is_clear(PIND,3))
    {
        // ----- Read the BMBT Buttons -----
        if (i2c_start(PORT_EXPANDER_BMBT + I2C_READ) == 0)
        {
            unsigned char state = ~(i2c_readNak());
            i2c_stop();

            g_buttons &= ~(BUTTON_LEFT_STATE_MASK);
            g_buttons |= ((uint32_t)state << 12);
        }
    }

    #if 1
    // interrupt was cleared, so we have read the right one
    if (bit_is_set(PIND,3)) goto nextkeys;

    // no button was previously down, so just get states of both parts and
    // try fix those part to GND which was responsible for the interrupt
    if (g_buttons_ExpanderState == 0)
    {
        // first read on the left side, mark if we were successfull
        ENABLE_RADIO_BUTTON_LEFT_PART();
        if (i2c_start(PORT_EXPANDER_RADIO + I2C_READ) == 0)
        {
            unsigned char state = ~(i2c_readNak());
            i2c_stop();
            state >>= 2;
            g_buttons &= ~(BUTTON_RIGHT_STATE_MASK_1);
            g_buttons |= ((uint32_t)state << 0);

            if (state != 0x00)
                g_buttons_ExpanderState = 2;
        }

        // nothing could be read on the left side, so read on the right side now
        if (g_buttons_ExpanderState == 0)
        {
            ENABLE_RADIO_BUTTON_RIGHT_PART();
            if (i2c_start(PORT_EXPANDER_RADIO + I2C_READ) == 0)
            {
                unsigned char state = ~(i2c_readNak());
                i2c_stop();
                state >>= 2;
                g_buttons &= ~(BUTTON_RIGHT_STATE_MASK_2);
                g_buttons |= ((uint32_t)state << 6);

                if (state != 0x00)
                    g_buttons_ExpanderState = 1;
            }
        }

        // if nothing helped, then interrupt came not from here -> restore old state
        if (g_buttons_ExpanderState == 0)
            ENABLE_RADIO_BUTTON_BOTH_PARTS();

    // we have previously pressed a button on the right side, so check if it is up now
    }else if (g_buttons_ExpanderState == 1)
    {
        if (i2c_start(PORT_EXPANDER_RADIO + I2C_READ) == 0)
        {
            unsigned char state = ~(i2c_readNak());
            i2c_stop();
            state >>= 2;
            g_buttons &= ~(BUTTON_RIGHT_STATE_MASK_2);
            g_buttons |= ((uint32_t)state << 6);

            // if all buttons on the right side are release, then go-to standard mode
            if (state == 0x00)
            {
                g_buttons_ExpanderState = 0;
                ENABLE_RADIO_BUTTON_BOTH_PARTS();
            }
        }

    // we are currently on the left state, so check if a button here is released
    }else if (g_buttons_ExpanderState == 2)
    {
        if (i2c_start(PORT_EXPANDER_RADIO + I2C_READ) == 0)
        {
            unsigned char state = ~(i2c_readNak());
            i2c_stop();
            state >>= 2;
            g_buttons &= ~(BUTTON_RIGHT_STATE_MASK_1);
            g_buttons |= ((uint32_t)state << 0);

            // if all buttons on the right side are release, then go-to standard mode
            if (state == 0x00)
            {
                g_buttons_ExpanderState = 0;
                ENABLE_RADIO_BUTTON_BOTH_PARTS();
            }
        }
    }

#endif

#if 0
        // ----- First read the Radio Part -----
        if (i2c_start(PORT_EXPANDER_RADIO + I2C_READ) == 0)
        {
            unsigned char state = ~(i2c_readNak());
            i2c_stop();

            // first two bits ignored
            state >>= 2;

            // setup buttons according to the state
            if (is_bit_set(g_buttons_last, BUTTON_ACTIVE_RIGHT_PART))
            {
                ENABLE_RADIO_BUTTON_RIGHT_PART();

                g_buttons &= ~(BUTTON_RIGHT_STATE_MASK_1);
                g_buttons |= ((uint32_t)state << 0);

                g_buttons &= ~(1L << BUTTON_ACTIVE_RIGHT_PART);
            }else
            {
                ENABLE_RADIO_BUTTON_LEFT_PART();
                g_buttons &= ~(BUTTON_RIGHT_STATE_MASK_2);
                g_buttons |= ((uint32_t)state << 6);

                g_buttons |= (1L << BUTTON_ACTIVE_RIGHT_PART);
            }
        }
#endif

nextkeys:
    // compute down, pressed and released states
    g_buttonsDown       = g_buttons_last    & g_buttons;
    g_buttonsPressed    = (~g_buttons_last) & g_buttons;
    g_buttonsRelease    = g_buttons_last    & (~g_buttons);

    // swap current with the old state
    g_buttons_last = g_buttons;

    // increment hold counter for all buttons, which are currently down
    uint8_t i;
    buttonGlobalState_t tempDownState = g_buttonsDown;
    for (i=0; i < BUTTON_NUM_BUTTONS; i++)
    {
        if (tempDownState & 1) g_button_delay[i]++;
        else g_button_delay[i] = 0;

        tempDownState >>= 1L;

        if (g_button_delay[i] > BUTTON_DELAY_LONG) g_button_delay[i] = 0;
    }
}

void button_isr(void)
{
    g_encoder_flag = 1;
}
