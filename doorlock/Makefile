obj-m += doorlock.o

KDIR := /lib/modules/$(shell uname -r)/build

PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

install:
	sudo insmod doorlock.ko
	sudo chmod 666 /dev/doorlock

uninstall:
	sudo rmmod doorlock

dmesg:
	dmesg | tail -20

.PHONY: all clean install uninstall test dmesg
