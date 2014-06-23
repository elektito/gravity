#ifndef _GRAVITY_RENDERER_HH_
#define _GRAVITY_RENDERER_HH_

#include "camera.hh"
#include "entity.hh"

#include <Box2D/Box2D.h>

#include <vector>
#include <string>

using namespace std;

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
  virtual void DrawText(string text,
                        SDL_Color color,
                        int scrx,
                        int scry,
                        bool anchorLeft=true,
                        bool anchorTop=true) const = 0;

  virtual void PresentScreen() = 0;
};

#endif /* _GRAVITY_RENDERER_HH_ */
