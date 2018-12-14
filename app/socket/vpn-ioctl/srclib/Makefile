

INCLUDEDIR=../include
CFLAGS=-lpthread -shared -Wall -g -I${INCLUDEDIR} 

EXTRAVERSION=$$$$
CFLAGS+=-DEXTRAVERSION=${EXTRAVERSION} 
CFLAGS+=${APP_VERSION} 


LIBDIR=../libs
LIBS1=${LIBDIR}/libsocketlib.so 
LIBS2=${LIBDIR}/libgpio.so 

TAR=${LIBS1} ${LIBS2}
OBJ1=socketlib.o 
OBJ2=cmd.o file_io.o getenv.o sktlist.o log.o bdversion.o  gpiointerface.o


all:${TAR}
	@echo ******************$(CFLAGS)************************	
${LIBS1}:${OBJ1}
	${CC} ${CFLAGS}   $^  -o ${LIBS1}	
${LIBS2}:${OBJ2}
	${CC} ${CFLAGS}   $^  -o ${LIBS2}	

clean:
	 rm -rf *.o  ${TAR}
