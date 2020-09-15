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
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/b8a8e5b6/objectc.o \
	${OBJECTDIR}/_ext/b8a8e5b6/unpack.o \
	${OBJECTDIR}/_ext/b8a8e5b6/version.o \
	${OBJECTDIR}/_ext/b8a8e5b6/zone.o \
	${OBJECTDIR}/microprop.o \
	${OBJECTDIR}/microprop_test.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lpthread

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/microprop

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/microprop: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/microprop ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/_ext/b8a8e5b6/objectc.o: ../msgpack-c/src/objectc.c
	${MKDIR} -p ${OBJECTDIR}/_ext/b8a8e5b6
	${RM} "$@.d"
	$(COMPILE.c) -g -I../msgpack-c/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/b8a8e5b6/objectc.o ../msgpack-c/src/objectc.c

${OBJECTDIR}/_ext/b8a8e5b6/unpack.o: ../msgpack-c/src/unpack.c
	${MKDIR} -p ${OBJECTDIR}/_ext/b8a8e5b6
	${RM} "$@.d"
	$(COMPILE.c) -g -I../msgpack-c/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/b8a8e5b6/unpack.o ../msgpack-c/src/unpack.c

${OBJECTDIR}/_ext/b8a8e5b6/version.o: ../msgpack-c/src/version.c
	${MKDIR} -p ${OBJECTDIR}/_ext/b8a8e5b6
	${RM} "$@.d"
	$(COMPILE.c) -g -I../msgpack-c/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/b8a8e5b6/version.o ../msgpack-c/src/version.c

${OBJECTDIR}/_ext/b8a8e5b6/zone.o: ../msgpack-c/src/zone.c
	${MKDIR} -p ${OBJECTDIR}/_ext/b8a8e5b6
	${RM} "$@.d"
	$(COMPILE.c) -g -I../msgpack-c/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/b8a8e5b6/zone.o ../msgpack-c/src/zone.c

${OBJECTDIR}/microprop.o: microprop.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I.. -I../.. -I../gtest/googletest -I../gtest/googletest/include -I../msgpack-c/include -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/microprop.o microprop.cpp

${OBJECTDIR}/microprop_test.o: microprop_test.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I.. -I../.. -I../gtest/googletest -I../gtest/googletest/include -I../msgpack-c/include -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/microprop_test.o microprop_test.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
