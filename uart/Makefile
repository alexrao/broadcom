KERN_DIR = /mnt/workspace/tools/stblinux-2.6.37/

PWD = $(shell pwd)

all:
	$(MAKE) -C $(KERN_DIR) M=$(PWD) modules
	
clean:
	$(MAKE) -C $(KERN_DIR) M=$(PWD) modules clean

obj-m += bcm_uart2.o
