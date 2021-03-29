#!/bin/bash

# This shell script can be used for building the Windows version of
# Gravity using MXE. It expects the location of MXE in an "MXE"
# environment variable and the target architecture in one named
# "ARCH". The location of MXE defaults to "~/source/mxe" while
# architecture defaults to "i686".
#
# After building the .exe, this will then create a portable
# distribution of the game in a directory with a name like
# "gravity-1.0.0-w32-portable". The version number can be changed by
# setting the environment variable VERSION.

VERSION=${VERSION:=1.0.0}
ARCH=${ARCH:=i686} # for 64 bit build, use x86_64
MXE=${MXE:=~/source/mxe}

export CXX=${MXE}/usr/bin/${ARCH}-w64-mingw32.static-g++
export LD=${MXE}/usr/bin/${ARCH}-w64-mingw32.static-ld
export AR=${MXE}/usr/bin/${ARCH}-w64-mingw32.static-ar
export PKGCONFIG=${MXE}/usr/bin/${ARCH}-w64-mingw32.static-pkg-config
export WINRC=${MXE}/usr/bin/${ARCH}-w64-mingw32.static-windres

./waf distclean configure --release --windows build
if (($? == 0)); then
    if [ "$ARCH" == "i686" ]; then
        WINVER=w32
    else
        WINVER=w64
    fi

    DIR=gravity-${VERSION}-${WINVER}-portable/
    mkdir -p $DIR
    cp build/gravity-bin.exe $DIR/
    cp -r resources/ $DIR/
    rm -f $DIR/resources/images/*.svg
    rm $DIR/resources/sources.txt
fi
