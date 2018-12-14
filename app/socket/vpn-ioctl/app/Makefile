

INCLUDEDIR=../include
LIBSDIR=../libs
BINSDIR=../bin
CFLAGS+=-lpthread -Wall -g -I${INCLUDEDIR}  -L${LIBSDIR} -lgpio  -lsocketlib  -ldpmsc

CFLAGS+=${DEVARCHFLAGS}
DEVPROCESSARCH=dev_process_${DEVARCH}.o

TAG=${BINSDIR}/sktgpio_server_${DEVARCH}  ${BINSDIR}/gpioclient_${DEVARCH}
all:${TAG}
	
${BINSDIR}/sktgpio_server_${DEVARCH}:sktgpio_server.o ${DEVPROCESSARCH}
	${CC} -o $@ $^ ${CFLAGS}

${BINSDIR}/gpioclient_${DEVARCH}:gpioclient.o
	${CC} -o $@ $^ ${CFLAGS}
clean:
	 rm -rf *.o  ${TAG}
