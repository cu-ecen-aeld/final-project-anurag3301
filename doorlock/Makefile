obj-m += doorlock.o

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) clean

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules_install

install:
	sudo insmod doorlock.ko
	sudo chmod 666 /dev/doorlock

uninstall:
	sudo rmmod doorlock

dmesg:
	dmesg | tail -20

.PHONY: all clean install uninstall test dmesg
