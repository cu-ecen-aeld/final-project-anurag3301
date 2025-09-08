obj-m += lcd1602.o

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) clean

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules_install

install:
	sudo insmod lcd1602.ko
	sudo chmod 666 /dev/lcd1602

uninstall:
	sudo rmmod lcd1602

dmesg:
	dmesg | tail -20

.PHONY: all clean install uninstall test dmesg
