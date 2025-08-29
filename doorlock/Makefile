obj-m += doorlock.o

# Kernel build directory (adjust if needed)
KDIR := /lib/modules/$(shell uname -r)/build

# Current directory
PWD := $(shell pwd)

# Default target
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# Clean target
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

# Install target (load the module)
install:
	sudo insmod doorlock.ko
	sudo chmod 666 /dev/doorlock

# Uninstall target (unload the module)
uninstall:
	sudo rmmod doorlock

dmesg:
	dmesg | tail -20

.PHONY: all clean install uninstall test dmesg
