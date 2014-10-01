Gravity: Friend or Foe?
=======================

**Gravity: Friend or Foe?** is a game in which you use gravity to move
objects and score.

In order to build _Gravity_ you need the following libraries:

 - SDL2
 - SDL2_ttf
 - SDL2_mixer
 - OpenGL
 - Box2D

In order to build run `./waf configure` and then `./waf build`. A
C++11 compliant compiler and a Python interpreter is needed.

My main target so far has been Linux, although cross-compiling for
Windows on Linux is possible by using [MXE](http://mxe.cc) (which
itself uses mingw-w64). Refer to `PACKAGING.crosswindows.md`.
