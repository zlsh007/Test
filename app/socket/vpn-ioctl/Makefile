ARCHS=arm
#ARCHS=X86
ifeq (${ARCHS},arm)
CC=arm-linux-gcc
else
CC=gcc
endif
include .config


all:
	@echo  ${EXTRAVERSION}
	make -C srclib
	make -C app

clean:
	make -C app  clean
	make -C srclib  clean
