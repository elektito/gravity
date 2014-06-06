#ifndef _GRAVITY_RENDERER_HH_
#define _GRAVITY_RENDERER_HH_

#include <Box2D/Box2D.h>

#include <vector>

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
  virtual void DrawBackground() = 0;
  virtual void DrawGrid() = 0;
  virtual void DrawEntities() = 0;
  virtual void DrawHud() = 0;
  virtual void PresentScreen() = 0;

  Renderer() :
    score(0)
  {
  }

public:
  int score;
  void Render() {
    this->DrawBackground();
    this->DrawGrid();
    this->DrawEntities();
    this->DrawHud();
    this->PresentScreen();
  }
};

#endif /* _GRAVITY_RENDERER_HH_ */
