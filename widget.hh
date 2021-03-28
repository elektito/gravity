#ifndef _GRAVITY_WIDGET_HH_
#define _GRAVITY_WIDGET_HH_

#include "renderer.hh"

#include <SDL2/SDL.h>

class Screen;

class Widget {
protected:
  Screen *screen;
  bool visible;

public:
  Widget(Screen *screen) :
    screen(screen),
    visible(true)
  {}

  void SetVisible(bool v) {
    this->visible = v;
  }

  bool GetVisible() const {
    return this->visible;
  }

  virtual void HandleEvent(const SDL_Event &e) = 0;
  virtual void Advance(float dt) = 0;
  virtual void Render(Renderer *renderer) = 0;
  virtual void Reset() = 0;

  virtual ~Widget() {}
};

#endif /* _GRAVITY_WIDGET_HH_ */
