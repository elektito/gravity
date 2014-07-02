#ifndef _GRAVITY_RENDERER_HH_
#define _GRAVITY_RENDERER_HH_

#include "camera.hh"
#include "entity.hh"

#include <Box2D/Box2D.h>

#include <vector>
#include <string>

using namespace std;

enum class TextAnchor {
  LEFT,
  RIGHT,
  TOP,
  BOTTOM,
  CENTER
};

class Renderer {
protected:
  Camera camera;

public:
  Renderer() = default;
  virtual ~Renderer() = default;

  void SetCamera(const Camera &camera) {
    this->camera = camera;
  }

  virtual void ClearScreen(int r, int g, int b) const = 0;
  virtual void DrawDisk(b2Vec2 pos, float32 radius, int r, int g, int b, int a) const = 0;
  virtual void DrawPolygon(b2Vec2 vertices[], int count) const = 0;
  virtual void DrawLine(b2Vec2 begin, b2Vec2 end, int r, int g, int b, int a) const = 0;

  /// Draws text at the given position, with the given height, both in
  /// world coordinates and units.
  virtual void DrawTextM(string text, b2Vec2 pos, float32 height, SDL_Color c) const = 0;

  /// Draws text at the given position, with the given height, both
  /// given as window size ratios (numbers in range [0.0, 1.0]).
  virtual void DrawTextP(string text,
                         float32 x,
                         float32 y,
                         float32 height,
                         SDL_Color c,
                         TextAnchor xanchor,
                         TextAnchor yanchor) const = 0;

  virtual void DrawTexture(SDL_Texture *texture,
                           b2Vec2 bottomLeft,
                           float32 width,
                           float32 height) const = 0;
  virtual void DrawBackground(SDL_Texture *texture) const = 0;

  virtual void PresentScreen() = 0;
};

#endif /* _GRAVITY_RENDERER_HH_ */
