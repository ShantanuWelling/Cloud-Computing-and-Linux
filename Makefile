# Makefile

obj-m += lkm1.o
obj-m += lkm2.o
obj-m += lkm3.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean