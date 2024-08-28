## ENV SETUP

### GET LINUX IMAGE

~~~
git clone git://git.ti.com/ti-linux-kernel/ti-linux-kernel.git linux --depth 1 --branch ti-linux-4.14.y
~~~

### BUILD DOCKER IMAGE 

~~~
cd docker
docker build -t bbb_bsp_img .
~~~

### START DOCKER CONTAINER WITH MOUNTPOINT

~~~
docker run -it --name bbb_bsp_dev -v /home/malinbay/workspace/bbb/:/home/ bbb_bsp_img
~~~

## BUILD

~~~
cd 001_hello_world_module
export CC=/home/malinbay/workspace/raspi3b-bsp/arm-buildroot-linux-gnueabihf_sdk-buildroot/bin/arm-none-linux-gnueabihf-
export KERNEL_DIR=/home/malinbay/workspace/raspi3b-bsp/linux/
make 
~~~
