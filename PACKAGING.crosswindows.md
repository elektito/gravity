Cross-Compiling for Windows on Linux
====================================

# Prepare the Build Environment

The first step is preparing a build environment for
cross-compiling. We will use MXE for this.

    # git clone https://github.com/mxe/mxe.git

Now go to the `mxe` directory. Before proceeding, update `src/sdl2.mk`
and `src/sdl2_mixer.mk` with the build options you want. At the time
of this writing, updating `sdl2_mixer.mk` is necessary because there
will be build failures when SDL2_mixer is built with SMPEG. You need
to either disable mp3 playback or use libmad instead.

Suggested SDL2 configuration options:

    --disable-atomic --disable-render --disable-joystick --disable-haptic --disable-power --disable-loadso --disable-cpuinfo --disable-dbus

Suggested SDL2_mixer configuration options:

    --disable-music-cmd --disable-music-mod --disable-music-midi --disable-music-ogg --disable-music-mp3 --disable-music-flac

After that, build the packages:

    # make MXE_TARGETS='i686-w64-mingw32.static' gcc sdl2 sdl2_mixer sdl2_ttf

This will build 32-bit versions of these packages. To build 64 bit
versions set MXE_TARGETS to `x86_646-w64-mingw32.static`.

# Build Box2D

Box2D is not available in MXE so we need to build it separately.

    # cd ~/source/box2d-2.3.1/
    # premake4 gmake
    # cd Build/gmake/
    # rm -rf obj/ bin/

Edit Box2D.make and change the lines that set CC, CXX and CR to these:

    CC = $(CROSS)gcc
    CXX = $(CROSS)g++
    AR = $(CROSS)ar

Then build and install Box2D like this:

    # make -f Box2D.make config=release CROSS=~/source/mxe/usr/bin/i686-w64-mingw32.static-
    # cp bin/Release/libBox2D.a ~/source/mxe/usr/i686-w64-mingw32.static/lib/
    # cd ../..
    # cp -r Box2D/ ~/source/mxe/usr/i686-w64-mingw32.static/include/

# Build Gravity

    # export CXX=~/source/mxe/usr/bin/i686-w64-mingw32.static-g++
    # export PKGCONFIG=~/source/mxe/usr/bin/i686-w64-mingw32.static-pkg-config
    # export WINRC=~/source/mxe/usr/bin/i686-w64-mingw32.static-windres
    # cd ~/source/gravity/
    # ./waf distclean configure --release --windows build

`gravity-bin.exe` will be created in the `build` directory.
