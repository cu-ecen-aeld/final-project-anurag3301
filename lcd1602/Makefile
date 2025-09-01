obj-m += lcd1602.o

KDIR := /lib/modules/$(shell uname -r)/build

PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

install:
	sudo insmod lcd1602.ko
	sudo chmod 666 /dev/lcd1602

uninstall:
	sudo rmmod lcd1602

dmesg:
	dmesg | tail -20

.PHONY: all clean install uninstall test dmesg
