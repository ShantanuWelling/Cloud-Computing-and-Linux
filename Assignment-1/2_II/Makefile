TARGETS = control_station soldier

SOURCES = $(TARGETS:=.c)
obj-m += ioctl_pid.o
KERNEL_DIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
all: $(TARGETS) ioctl_pid.ko

$(TARGETS): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $@.c

ioctl_pid.ko:
	make -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	rm -f $(TARGETS)
	make -C $(KERNEL_DIR) M=$(PWD) clean

