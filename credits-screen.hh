#ifndef _GRAVITY_CREDITS_SCREEN_HH_
#define _GRAVITY_CREDITS_SCREEN_HH_

#include "screen.hh"
#include "image-button-widget.hh"

class CreditsScreen : public Screen {
protected:
  Background background;
  ImageWidget *creditsImage;

public:
  CreditsScreen(SDL_Window *window);
  virtual ~CreditsScreen();

  virtual void SwitchScreen(const map<string, string> &lastState);
  virtual void HandleEvent(const SDL_Event &e);
  virtual void HandleWidgetEvent(int event_type, Widget *widget);

  virtual void Reset();
  virtual void Save(ostream &s) const;
  virtual void Load(istream &s);

  virtual void Advance(float dt);
  virtual void Render(Renderer *renderer);
};

#endif /* _GRAVITY_CREDITS_SCREEN_HH_ */
