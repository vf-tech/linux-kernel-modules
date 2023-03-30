## ENV SETUP

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
cd lkm-1
make 
~~~
