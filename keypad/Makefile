obj-m += keypad.o

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) clean

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules_install

install:
	sudo insmod keypad.ko
	sudo chmod 666 /dev/keypad

uninstall:
	sudo rmmod keypad

dmesg:
	dmesg | tail -20

.PHONY: all clean install uninstall test dmesg
