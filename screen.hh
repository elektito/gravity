#ifndef _GRAVITY_SCREEN_HH_
#define _GRAVITY_SCREEN_HH_

#include "renderer.hh"

#include <SDL2/SDL.h>

#include <iostream>
#include <map>

using namespace std;

class Screen {
protected:
  SDL_Window *window;

public:
  Screen(SDL_Window *window) :
    window(window)
  {}

  virtual ~Screen() = default;

  map<string, string> state;

  virtual void SwitchScreen(const map<string, string> &lastState) = 0;
  virtual void HandleEvent(const SDL_Event &e) = 0;

  virtual void Reset() = 0;
  virtual void Save(ostream &s) const = 0;
  virtual void Load(istream &s) = 0;

  virtual void Advance(float dt) = 0;
  virtual void Render(Renderer *renderer) = 0;
};

#endif /* _GRAVITY_SCREEN_HH_ */
