#ifndef _GRAVITY_WIDGET_HH_
#define _GRAVITY_WIDGET_HH_

#include "renderer.hh"

#include <SDL2/SDL.h>

class Screen;

class Widget {
protected:
  Screen *screen;

public:
  Widget(Screen *screen) :
    screen(screen)
  {}

  virtual void HandleEvent(const SDL_Event &e) = 0;
  virtual void Advance(float dt) = 0;
  virtual void Render(Renderer *renderer) = 0;
};

#endif /* _GRAVITY_WIDGET_HH_ */
