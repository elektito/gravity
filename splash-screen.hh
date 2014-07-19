#ifndef _GRAVITY_SPLASH_HH_
#define _GRAVITY_SPLASH_HH_

#include "screen.hh"

class SplashScreen : public Screen {
protected:
  Background background;
public:
  SplashScreen(SDL_Window *window);
  virtual ~SplashScreen();

  virtual void SwitchScreen(const map<string, string> &lastState);
  virtual void HandleEvent(const SDL_Event &e);

  virtual void Reset();
  virtual void Save(ostream &s) const;
  virtual void Load(istream &s);

  virtual void Advance(float dt);
  virtual void Render(Renderer *renderer);
};

#endif /* _GRAVITY_SPLASH_HH_ */
