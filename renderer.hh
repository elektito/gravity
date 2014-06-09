#ifndef _GRAVITY_RENDERER_HH_
#define _GRAVITY_RENDERER_HH_

#include "camera.hh"
#include "entity.hh"

#include <Box2D/Box2D.h>

#include <vector>
#include <string>

using namespace std;

struct TrailPoint {
  b2Vec2 pos;
  float32 time;
};

struct Trail {
  b2Body *body;
  int size;
  float32 time;
  vector<TrailPoint> points;
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

  virtual void DrawBackground() const = 0;
  virtual void DrawGrid() const = 0;
  virtual void DrawEntity(const Entity *entity) const = 0;
  virtual void DrawTrail(const Trail *t) const = 0;
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
