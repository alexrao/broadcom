# author rxp
PWD = $(shell pwd)
KERNEL_DIR = /mnt/workspace/tools/stblinux-2.6.37/

all:
	$(MAKE)  -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	$(MAKE)  -C $(KERNEL_DIR) M=$(PWD) modules clean
	rm -rf modules.order

obj-m += helloworld.o
