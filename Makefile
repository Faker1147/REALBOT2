CPP = g++

ARCHFLAG = -m32

METAMOD_SRCDIR = ./dependencies/metamod-hl1/metamod
HLSDK_BASEDIR = ./dependencies/hlsdk

BASEFLAGS = -Dstricmp=strcasecmp -Dstrcmpi=strcasecmp -Dlinux=1
CPPFLAGS = ${BASEFLAGS} ${ARCHFLAG} -O2 -w -fpermissive -std=gnu++98 \
	-I"${METAMOD_SRCDIR}" \
	-I"${HLSDK_BASEDIR}/common" \
	-I"${HLSDK_BASEDIR}/dlls" \
	-I"${HLSDK_BASEDIR}/engine" \
	-I"${HLSDK_BASEDIR}/pm_shared" \
	-I"${HLSDK_BASEDIR}/public"

OBJ = NodeMachine.o \
	bot.o \
	bot_buycode.o \
	bot_client.o \
	bot_func.o \
	bot_navigate.o \
	build.o \
	dll.o \
	engine.o \
	game.o \
	util.o \
	ChatEngine.o \
	IniParser.o \
	menu.o

TARGET = realbot2_mm_i386.so

all: ${TARGET}

${TARGET}: ${OBJ}
	${CPP} ${ARCHFLAG} -fPIC -shared -o $@ ${OBJ} -ldl -lm

clean:
	rm -f *.o
	rm -f ${TARGET}

distclean: clean

%.o:	%.cpp
	${CPP} ${CPPFLAGS} -fPIC -c $< -o $@

%.o:	%.c
	${CPP} ${CPPFLAGS} -fPIC -c $< -o $@
