INCLUDEDIR=../include
LIBSDIR=../lib
BINSDIR=../bin
CFLAGS+=-lpthread -g -I${INCLUDEDIR} -L${LIBSDIR} -lsocket -lcmd -lsktlist
EXTRA_CFLAGS = -Wno-unused-parameter

TAG=${BINSDIR}/server  ${BINSDIR}/client
all:${TAG}
	
${BINSDIR}/server:server.o
	${CC} -o $@ $^ ${CFLAGS} ${EXTRA_CFLAGS}

${BINSDIR}/client:client.o
	${CC} -o $@ $^ ${CFLAGS} ${EXTRA_CFLAGS}
clean:
	 rm -rf *.o  ${BINSDIR}/*
