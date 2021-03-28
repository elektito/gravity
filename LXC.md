1. Create an LXC container.

        sudo lxc-create -t ubuntu -n gravity

2. Edit the container configuration file (/var/lib/lxc/gravity/config)
   and add these lines:

        lxc.cgroup.devices.allow = c 1:1 rwm # dev/mem
        lxc.cgroup.devices.allow = c 13:63 rwm # dev/input/mice
        lxc.cgroup.devices.allow = c 195:* rwm # dev/nvidia*

3. Edit the container fstab file (/var/lib/lxc/gravity/fstab) and add
   these lines:

        /dev/dri dev/dri none bind,optional,create=dir
        /dev/snd dev/snd none bind,optional,create=dir
        /tmp/.X11-unix tmp/.X11-unix none bind,optional,create=dir
        /dev/video0 dev/video0 none bind,optional,create=file
        /dev/nvidiactl dev/nvidiactl none bind,optional,create=file
        /dev/nvidia0 dev/nvidia0 none bind,optional,create=file

4. Copy source code into the container:

        sudo cp -r /path/to/source/code/of/gravity /var/lib/lxc/gravity/rootfs/home/ubuntu/
        sudo cp /path/to/box2d-2.4.1.tar.xz /var/lib/lxc/gravity/rootfs/home/ubuntu/

5. Start the container.

        sudo lxc-start -n gravity -d

6. Attach to the container (login with the user ubuntu and password
   ubuntu):

        sudo lxc-console -n gravity

7. Set apt proxy if necessary. Create a /etc/apt/apt.conf.d/90proxy
   and put something like this in it:

        Acquire::http { Proxy "http://10.0.3.1:3142"; };

8. Install the needed tools and libraries:

        sudo apt-get install python build-essential pulseaudio alsa libsdl2-dev libsdl2-mixer-dev libsdl2-ttf-dev libglew-dev nvidia-current

9. Load PulseAudio TCP module by running the following command on the
    _host_:

        pactl load-module module-native-protocol-tcp

10. Set the PulseAudio server in the guest:

        export PULSE_SERVER=10.0.3.1:4713

11. Set the X DISPLAY environment variable:

        export DISPLAY=:0.0

12. Correct the ownership of the source code directory:

        sudo chown ubuntu ~/gravity/ -R

13. Build and install Box2D:

        cd
        tar -xf box2d-2.4.1.tar.xz
        cd box2d-2.4.1/
        ./build.sh
        sudo cp build/bin/libbox2d.a /usr/local/lib/
        cd ../..
        sudo cp -r box2d/ /usr/local/include/

14. Build the game:

        cd ~/gravity/
        ./waf configure --release
        ./waf build

15. Run the game:

        ./build/gravity
