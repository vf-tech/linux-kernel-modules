obj-m := pcd_platform_driver.o pcd_device_setup.o
ARCH=arm
KERNEL_DIR ?= ../linux/
CROSS_COMPILE ?= ../buildroot-2018.08/output/host/bin/arm-linux-gnueabihf-

all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNEL_DIR) M=$(PWD) clean

