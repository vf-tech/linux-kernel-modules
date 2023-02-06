obj-m := main.o
KERNEL_DIR := /home/linux/
CC_ARM = /home/buildroot/output/host/bin/arm-linux-gnueabihf-

all:
	make ARCH=arm CROSS_COMPILE=$(CC_ARM) -C $(KERNEL_DIR) M=$(PWD) modules


clean:
	make ARCH=arm CROSS_COMPILE=$(CC_ARM) -C $(KERNEL_DIR) M=$(PWD) clean
