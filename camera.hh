#ifndef _GRAVITY_CAMERA_HH_
#define _GRAVITY_CAMERA_HH_

#include <box2d/box2d.h>
#include <SDL2/SDL.h>

struct Camera {
  Camera();

  /// The bottom-left position of the camera.
  b2Vec2 pos;

  /// Pixels-per-meter
  float ppm;

  /// Convert the given point in world coordinates to screen
  /// coordinate seen in this camera. The `x` and `y` parameters will
  /// hold the converted coordinates upon return.
  void PointToScreen(SDL_Window *window, b2Vec2 p, int &x, int &y) const;

  /// Convert the given length in world units to pixels seen by this
  /// camera.
  float LengthToScreen(float length) const;

  /// Convert the given point (x, y) on screen to world coordinates.
  b2Vec2 PointToWorld(int x, int y, SDL_Window *window) const;

  /// Convert the given length in pixels to world units.
  float LengthToWorld(float length) const;
};

#endif /* _GRAVITY_CAMERA_HH_ */
