Cross-Compiling for Windows on Linux
====================================

# Prepare the Build Environment

The first step is preparing a build environment for
cross-compiling. We will use MXE for this.

    $ git clone https://github.com/mxe/mxe.git

Now go to the `mxe` directory. Before proceeding, update `src/sdl2.mk`
and `src/sdl2_mixer.mk` with the build options you want. At the time
of this writing, updating `sdl2_mixer.mk` is necessary because there
will be build failures when SDL2_mixer is built with SMPEG. You need
to either disable mp3 playback or use libmad instead.

Suggested SDL2 configuration options:

    --disable-atomic --disable-render --disable-joystick --disable-haptic --disable-power --disable-loadso --disable-cpuinfo --disable-dbus --disable-vulkan

Suggested SDL2_mixer configuration options:

    --disable-music-cmd --disable-music-mod --disable-music-midi --disable-music-ogg --disable-music-mp3 --disable-music-flac --disable-music-opus --disable-music-mp3-mpg123

Check `src/box2d.mk`. If the version (like the time of this writing)
is lower than 2.4.1, replace its contents with the following:

    PKG             := box2d
    $(PKG)_WEBSITE  := https://www.box2d.org/
    $(PKG)_IGNORE   :=
    $(PKG)_VERSION  := 2.4.1
    $(PKG)_CHECKSUM := d6b4650ff897ee1ead27cf77a5933ea197cbeef6705638dd181adc2e816b23c2
    $(PKG)_GH_CONF  := erincatto/box2d/tags, v
    $(PKG)_DEPS     := cc

    define $(PKG)_BUILD
        cd '$(BUILD_DIR)' && $(TARGET)-cmake \
            -DBOX2D_INSTALL=ON \
            -DBOX2D_BUILD_TESTBED=OFF \
            -DBOX2D_BUILD_DOCS=OFF \
            -DBOX2D_BUILD_UNIT_TESTS=OFF \
            -DBOX2D_BUILD_STATIC=$(CMAKE_STATIC_BOOL) \
            -DBOX2D_BUILD_SHARED=$(CMAKE_SHARED_BOOL) \
            '$(SOURCE_DIR)'
        $(MAKE) -C '$(BUILD_DIR)' -j '$(JOBS)' VERBOSE=1
        $(MAKE) -C '$(BUILD_DIR)' -j 1 install VERBOSE=1
    endef

After that, build the packages:

    $ make MXE_TARGETS='i686-w64-mingw32.static' gcc sdl2 sdl2_mixer sdl2_ttf box2d

This will build 32-bit versions of these packages. To build 64 bit
versions set MXE_TARGETS to `x86_64-w64-mingw32.static`.

# Build Gravity

    $ export CXX=~/source/mxe/usr/bin/i686-w64-mingw32.static-g++
    $ export PKGCONFIG=~/source/mxe/usr/bin/i686-w64-mingw32.static-pkg-config
    $ export WINRC=~/source/mxe/usr/bin/i686-w64-mingw32.static-windres
    $ cd ~/source/gravity/
    $ ./waf distclean configure --release --windows --installer build

`gravity-bin.exe` will be created in the `build` directory.
