#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/Application.o \
	${OBJECTDIR}/_ext/2043845659/mongoose.o \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/Log.o \
	${OBJECTDIR}/ApplicationClient.o \
	${OBJECTDIR}/IBus.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-pthread
CXXFLAGS=-pthread

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=/usr/lib/libboost_date_time.a /usr/lib/libboost_system-mt.a /usr/lib/libboost_thread-mt.a /usr/lib/libboost_program_options-mt.a -ldl

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/openbm-gateway

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/openbm-gateway: /usr/lib/libboost_date_time.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/openbm-gateway: /usr/lib/libboost_system-mt.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/openbm-gateway: /usr/lib/libboost_thread-mt.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/openbm-gateway: /usr/lib/libboost_program_options-mt.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/openbm-gateway: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/openbm-gateway -s ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/Application.o: Application.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -Werror -s -MMD -MP -MF $@.d -o ${OBJECTDIR}/Application.o Application.cpp

${OBJECTDIR}/_ext/2043845659/mongoose.o: /media/sda6/home/tevs/src/psyBMW_trunk/OpenBM-Gateway/mongoose.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/2043845659
	${RM} $@.d
	$(COMPILE.c) -O3 -Werror -s -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/2043845659/mongoose.o /media/sda6/home/tevs/src/psyBMW_trunk/OpenBM-Gateway/mongoose.c

${OBJECTDIR}/main.o: main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -Werror -s -MMD -MP -MF $@.d -o ${OBJECTDIR}/main.o main.cpp

${OBJECTDIR}/Log.o: Log.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -Werror -s -MMD -MP -MF $@.d -o ${OBJECTDIR}/Log.o Log.cpp

${OBJECTDIR}/ApplicationClient.o: ApplicationClient.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -Werror -s -MMD -MP -MF $@.d -o ${OBJECTDIR}/ApplicationClient.o ApplicationClient.cpp

${OBJECTDIR}/IBus.o: IBus.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -Werror -s -MMD -MP -MF $@.d -o ${OBJECTDIR}/IBus.o IBus.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/openbm-gateway

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc