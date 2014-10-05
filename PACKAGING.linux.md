General Packaging Guide for Linux
=================================

This guide contains the instructions for packaging the game for Linux
and testing the resulting package. In order to have a clean and
isolated environment for building and testing the game, we will use
Linux Containers.

I've used Ubuntu (14.04) for both the host system and the containers,
but it should not be too difficult to adopt the instructions for other
distributions.

The package that we will create in the end should run on any modern
Linux system.

Building in a container
=======================

Create a container (if you want to create a container with a different
architecture than the host system, add something like ` --
--arch=i386` to the end of this line):

    $ sudo lxc-create -t ubuntu -n build-container

Copy the gravity source code and the necessary packages into the
container:

    $ cd ~/source/
    $ sudo git clone ./gravity/ /var/lib/lxc/build-container/rootfs/home/ubuntu/gravity/
    $ sudo cp box2d-2.3.1.tar.xz freetype-2.5.3.tar.bz2 SDL2-2.0.3.tar.gz SDL2_mixer-2.0.0.tar.gz SDL2_ttf-2.0.12.tar.gz /var/lib/lxc/build-container/rootfs/home/ubuntu/

Copy premake4 into the container (notice that if you are building a
container with a different architecture than the host system, you need
to make a premake4 binary of appropriate architecture):

    $ sudo cp /usr/bin/premake4 /var/lib/lxc/build-container/rootfs/usr/bin/

Set the container APT proxy if necessary:

    $ echo 'Acquire::http { Proxy "http://10.0.3.1:3142"; };' | sudo tee /var/lib/lxc/build-container/rootfs/etc/apt/apt.conf.d/90proxy > /dev/null

Start the container and log into it:

    $ sudo lxc-start -d -n build-container
    $ sudo lxc-console -n build-container

Inside the container:

    $ sudo apt-get update
    $ sudo apt-get install python build-essential nvidia-current libpulse-dev libasound2-dev xorg-dev libglu1-mesa-dev
    $ sudo chown ubuntu -R ~/
    $ tar -xf box2d-2.3.1.tar.xz
    $ tar -xf freetype-2.5.3.tar.bz2
    $ tar -xf SDL2-2.0.3.tar.gz
    $ tar -xf SDL2_mixer-2.0.0.tar.gz
    $ tar -xf SDL2_ttf-2.0.12.tar.gz
    $ cd box2d-2.3.1/Box2D/
    $ premake4 gmake
    $ cd Build/gmake/
    $ make -f Box2D.make config=release
    $ sudo cp bin/Release/libBox2D.a /usr/local/lib/
    $ cd ../../
    $ sudo cp -r Box2D /usr/local/include/
    $ cd ../../freetype-2.5.3/
    $ ./configure
    $ make
    $ sudo make install
    $ cd ../SDL2-2.0.3/
    $ ./configure --disable-atomic --disable-render --disable-joystick --disable-haptic --disable-power --disable-loadso --disable-cpuinfo --disable-dbus
    $ make
    $ sudo make install
    $ cd ../SDL2_ttf-2.0.12/
    $ ./configure
    $ make
    $ sudo make install
    $ cd ../SDL2_mixer-2.0.0/
    $ ./configure --disable-music-cmd --disable-music-mod --disable-music-midi --disable-music-ogg --disable-music-mp3 --disable-music-flac
    $ make
    $ sudo make install
    $ cd ../gravity/
    $ ./waf configure --release
    $ ./waf build
    $ cd ..
    $ mkdir gravity-1.0
    $ cd gravity-1.0/
    $ cp ../gravity/build/gravity-bin .
    $ cp -r ../gravity/resources/ .
    $ rm resources/images/*.svg
    $ rm resources/sources.txt
    $ mkdir lib
    $ cp /usr/local/lib/libSDL2-2.0.so.0.2.1 ./lib/libSDL2-2.0.so.0
    $ cp /usr/local/lib/libfreetype.so.6.11.2 ./lib/libfreetype.so.6.11.2
    $ cp /usr/local/lib/libSDL2_ttf-2.0.so.0.10.2 ./lib/libSDL2_ttf-2.0.so.0
    $ cp /usr/local/lib/libSDL2_mixer-2.0.so.0.0.0 ./lib/libSDL2_mixer-2.0.so.0
    $ strip ./lib/*
    $ echo \#\!/bin/sh > launch.sh
    $ echo LD_LIBRARY_PATH=./lib ./gravity-bin >> launch.sh
    $ chmod +x launch.sh
    $ cd ..
    $ tar -czf gravity-1.0.tar.gz gravity-1.0/

Running in a container
======================

Create another container:

    $ sudo lxc-create -t ubuntu -n run-container

Edit the container configuration file
(/var/lib/lxc/run-container/config) and add these lines:

    lxc.cgroup.devices.allow = c 1:1 rwm # dev/mem
    lxc.cgroup.devices.allow = c 13:63 rwm # dev/input/mice
    lxc.cgroup.devices.allow = c 195:* rwm # dev/nvidia*

Edit the container fstab file (/var/lib/lxc/run-container/fstab) and
add these lines:

    /dev/dri dev/dri none bind,optional,create=dir
    /dev/snd dev/snd none bind,optional,create=dir
    /tmp/.X11-unix tmp/.X11-unix none bind,optional,create=dir
    /dev/video0 dev/video0 none bind,optional,create=file
    /dev/nvidiactl dev/nvidiactl none bind,optional,create=file
    /dev/nvidia0 dev/nvidia0 none bind,optional,create=file

Set the container APT proxy if necessary:

    $ echo 'Acquire::http { Proxy "http://10.0.3.1:3142"; };' | sudo tee /var/lib/lxc/run-container/rootfs/etc/apt/apt.conf.d/90proxy > /dev/null

Copy the distribution package from the other container into the new
container:

    $ sudo cp /var/lib/lxc/build-container/rootfs/home/ubuntu/gravity-1.0.tar.gz /var/lib/lxc/run-container/rootfs/home/ubuntu/

Copy PulseAudio cookie into the container:

    $ sudo mkdir -p /var/lib/lxc/run-container/rootfs/home/ubuntu/.config/pulse/
    $ sudo cp ~/.config/pulse/cookie /var/lib/lxc/run-container/rootfs/home/ubuntu/.config/pulse/

Enable the PulseAudio TCP module:

    $ pactl load-module module-native-protocol-tcp

Add these lines to the /var/lib/lxc/run-container/rootfs/home/ubuntu/.bashrc

    export DISPLAY=:0.0
    export PULSE_SERVER=10.0.3.1:4713

Start the container and log into it:

    $ sudo lxc-start -d -n run-container
    $ sudo lxc-console -n run-container

Inside the container:

    $ sudo apt-get update
    $ sudo apt-get install pulseaudio nvidia-current
    $ sudo chown ubuntu -R ~/
    $ tar -xf gravity-1.0.tar.gz
    $ cd gravity-1.0/
    $ ./launch.sh
