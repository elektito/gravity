#!/bin/bash

# This shell script can be used for building the Windows version of
# Gravity using MXE. It expects the location of MXE in an "MXE"
# environment variable and the target architecture in one named
# "ARCH". The location of MXE defaults to "~/source/mxe" while
# architecture defaults to "i686".

ARCH=${ARCH:=i686}
MXE=${MXE:=~/source/mxe}

export CXX=${MXE}/usr/bin/${ARCH}-w64-mingw32.static-g++
export LD=${MXE}/usr/bin/${ARCH}-w64-mingw32.static-ld
export AR=${MXE}/usr/bin/${ARCH}-w64-mingw32.static-ar
export PKGCONFIG=${MXE}/usr/bin/${ARCH}-w64-mingw32.static-pkg-config
export WINRC=${MXE}/usr/bin/${ARCH}-w64-mingw32.static-windres

./waf distclean configure --windows build
