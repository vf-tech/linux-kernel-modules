obj-m := main.o
KERNEL_DIR := ../linux/
CC_ARM = ../buildroot/output/host/bin/arm-linux-gnueabihf-

all:
	make ARCH=arm CROSS_COMPILE=$(CC_ARM) -C $(KERNEL_DIR) M=$(PWD) modules


clean:
	make ARCH=arm CROSS_COMPILE=$(CC_ARM) -C $(KERNEL_DIR) M=$(PWD) clean
