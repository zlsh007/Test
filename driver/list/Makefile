obj-m +=test.o
KVER := /lib/modules/`uname -r`/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KVER) M=$(PWD) modules
clean:
	$(MAKE) -C $(KVER) M=$(PWD) clean
install:
	insmod ./cache.ko 
	echo /dev/sdb 1 1 0 > /sys/cache/luns/add
uninstall:
	echo /dev/sdb > /sys/cache/luns/del
	rmmod cache

