obj-m := pcd.o
ARCH=arm
KERNEL_DIR ?= ../linux/
CC_ARM ?= ../buildroot-2018.08/output/host/bin/arm-linux-gnueabihf-

all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CC_ARM) -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CC_ARM) -C $(KERNEL_DIR) M=$(PWD) clean

