/* 
 * File:   emul_mid.h
 * Author: tevs
 *
 * Emulation of MID interface
 * 
 * Created on June 4, 2010, 12:12 AM
 */

#ifndef _EMUL_MID_H
#define	_EMUL_MID_H

#ifdef	__cplusplus
extern "C"
{
#endif

void emul_mid_init(void);

/**
 * Run MID emulation task. This will check for buttons and execute corresponding actions
 **/
extern void emul_mid_tick(void);
extern void emul_mid_encoder_tick(void);

//extern void emul_openbm_tick(void);
//extern void emul_bmbt_tick(void);

extern void emul_mid_on_bus_msg(uint8_t src, uint8_t dst, uint8_t* msg, uint8_t msglen);
//extern void emul_openbm_on_bus_msg(uint8_t src, uint8_t dst, uint8_t* msg, uint8_t msglen);
//extern void emul_bmbt_on_bus_msg(uint8_t src, uint8_t dst, uint8_t* msg, uint8_t msglen);

#ifdef	__cplusplus
}
#endif

#endif	/* _EMUL_MID_H */

