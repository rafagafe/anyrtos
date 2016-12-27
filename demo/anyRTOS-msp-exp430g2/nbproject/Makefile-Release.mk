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
CC=msp430-gcc
CCC=msp430-g++
CXX=msp430-g++
FC=gfortran
AS=msp430-as

# Macros
CND_PLATFORM=msp430-Linux
CND_DLIB_EXT=so
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/925292fd/queue.o \
	${OBJECTDIR}/_ext/4b93847/anyRTOS.o \
	${OBJECTDIR}/_ext/dbb3556f/adc.o \
	${OBJECTDIR}/_ext/dbb3556f/board-msp-exp430g2.o \
	${OBJECTDIR}/_ext/dbb3556f/serial-port.o \
	${OBJECTDIR}/_ext/dbb3556f/timers.o \
	${OBJECTDIR}/src/date-time.o \
	${OBJECTDIR}/src/main.o


# C Compiler Flags
CFLAGS=-mmcu=msp430g2553 -ffunction-sections

# CC Compiler Flags
CCFLAGS=-mmcu=msp430g2553
CXXFLAGS=-mmcu=msp430g2553

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/anyRTOS-sample.elf

${CND_DISTDIR}/${CND_CONF}/anyRTOS-sample.elf: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}
	${LINK.c} -o ${CND_DISTDIR}/${CND_CONF}/anyRTOS-sample.elf ${OBJECTFILES} ${LDLIBSOPTIONS} -Wl,--gc-sections

${OBJECTDIR}/_ext/925292fd/queue.o: ../../anyRTOS-util/queue.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/925292fd
	${RM} "$@.d"
	$(COMPILE.c) -O3 -Werror -D__MSP430G2553__ -I../../anyRTOS -I../../anyRTOS-util -I./src -I../foundation -std=c99 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/925292fd/queue.o ../../anyRTOS-util/queue.c

${OBJECTDIR}/_ext/4b93847/anyRTOS.o: ../../anyRTOS/src/anyRTOS.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/4b93847
	${RM} "$@.d"
	$(COMPILE.c) -O3 -Werror -D__MSP430G2553__ -I../../anyRTOS -I../../anyRTOS-util -I./src -I../foundation -std=c99 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/4b93847/anyRTOS.o ../../anyRTOS/src/anyRTOS.c

${OBJECTDIR}/_ext/dbb3556f/adc.o: ../foundation/msp-exp430g2/adc.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/dbb3556f
	${RM} "$@.d"
	$(COMPILE.c) -O3 -Werror -D__MSP430G2553__ -I../../anyRTOS -I../../anyRTOS-util -I./src -I../foundation -std=c99 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/dbb3556f/adc.o ../foundation/msp-exp430g2/adc.c

${OBJECTDIR}/_ext/dbb3556f/board-msp-exp430g2.o: ../foundation/msp-exp430g2/board-msp-exp430g2.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/dbb3556f
	${RM} "$@.d"
	$(COMPILE.c) -O3 -Werror -D__MSP430G2553__ -I../../anyRTOS -I../../anyRTOS-util -I./src -I../foundation -std=c99 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/dbb3556f/board-msp-exp430g2.o ../foundation/msp-exp430g2/board-msp-exp430g2.c

${OBJECTDIR}/_ext/dbb3556f/serial-port.o: ../foundation/msp-exp430g2/serial-port.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/dbb3556f
	${RM} "$@.d"
	$(COMPILE.c) -O3 -Werror -D__MSP430G2553__ -I../../anyRTOS -I../../anyRTOS-util -I./src -I../foundation -std=c99 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/dbb3556f/serial-port.o ../foundation/msp-exp430g2/serial-port.c

${OBJECTDIR}/_ext/dbb3556f/timers.o: ../foundation/msp-exp430g2/timers.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/dbb3556f
	${RM} "$@.d"
	$(COMPILE.c) -O3 -Werror -D__MSP430G2553__ -I../../anyRTOS -I../../anyRTOS-util -I./src -I../foundation -std=c99 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/dbb3556f/timers.o ../foundation/msp-exp430g2/timers.c

${OBJECTDIR}/src/date-time.o: src/date-time.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O3 -Werror -D__MSP430G2553__ -I../../anyRTOS -I../../anyRTOS-util -I./src -I../foundation -std=c99 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/date-time.o src/date-time.c

${OBJECTDIR}/src/main.o: src/main.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O3 -Werror -D__MSP430G2553__ -I../../anyRTOS -I../../anyRTOS-util -I./src -I../foundation -std=c99 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/main.o src/main.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/anyRTOS-sample.elf

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
