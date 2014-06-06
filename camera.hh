#ifndef _GRAVITY_CAMERA_HH_
#define _GRAVITY_CAMERA_HH_

#include <Box2D/Box2D.h>
#include <SDL2/SDL.h>

struct Camera {
  /// The bottom-left position of the camera.
  b2Vec2 pos;

  /// Pixels-per-meter
  float32 ppm;

  /// Convert the given point in world coordinates to screen
  /// coordinate seen in this camera. The `x` and `y` parameters will
  /// hold the converted coordinates upon return.
  void PointToScreen(SDL_Window *window, b2Vec2 p, int &x, int &y);

  /// Convert the given length in world units to pixels seen by this
  /// camera.
  float32 LengthToScreen(float32 length);

  /// Convert the given point (x, y) on screen to world coordinates.
  b2Vec2 PointToWorld(int x, int y, SDL_Window *window);

  /// Convert the given length in pixels to world units.
  float32 LengthToWorld(float32 length);
};

#endif /* _GRAVITY_CAMERA_HH_ */
