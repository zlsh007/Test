INCLUDEDIR=../include
CFLAGS=-lpthread -shared -Wall -g -I${INCLUDEDIR}

LIBDIR=../lib
LIB1=${LIBDIR}/libsocket.so
LIB2=${LIBDIR}/libcmd.so
LIB3=${LIBDIR}/libsktlist.so

TAR=${LIB1} ${LIB2} ${LIB3}
OBJ1=socketlib.o
OBJ2=cmd.o
OBJ3=sktlist.o

all:${TAR}
${LIB1}:${OBJ1}
	${CC} ${CFLAGS} $^ -o ${LIB1}
${LIB2}:${OBJ2}
	${CC} ${CFLAGS} $^ -o ${LIB2}
${LIB3}:${OBJ3}
	${CC} ${CFLAGS} $^ -o ${LIB3}

clean:
	 rm -rf *.o  ${TAR}
