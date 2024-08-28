obj-m := pcd_platform_driver.o pcd_device_setup.o
ARCH=arm
KERNEL_DIR ?= ../linux/
CC ?= ../buildroot-2018.08/output/host/bin/arm-linux-gnueabihf-

all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CC) -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CC) -C $(KERNEL_DIR) M=$(PWD) clean

