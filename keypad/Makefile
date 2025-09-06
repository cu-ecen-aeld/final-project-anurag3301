obj-m += keypad.o

KDIR := /lib/modules/$(shell uname -r)/build

PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

install:
	sudo insmod keypad.ko
	sudo chmod 666 /dev/keypad

uninstall:
	sudo rmmod keypad

dmesg:
	dmesg | tail -20

.PHONY: all clean install uninstall test dmesg
