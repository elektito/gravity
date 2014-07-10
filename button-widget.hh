#ifndef _GRAVITY_BUTTON_WIDGET_HH_
#define _GRAVITY_BUTTON_WIDGET_HH_

#include "label-widget.hh"

#include <string>

using namespace std;

#define BUTTON_CLICK 1000

class ButtonWidget : public LabelWidget {
protected:
  SDL_Color activeColor;
  SDL_Color inactiveColor;

  bool isActive;
  bool mouseDown;

public:
  ButtonWidget(Screen *screen,
               string text,
               float x,
               float y,
               float height,
               TextAnchor xanchor,
               TextAnchor yanchor,
               const SDL_Color &activeColor,
               const SDL_Color &inactiveColor) :
    LabelWidget(screen, text, x, y, height, xanchor, yanchor, inactiveColor),
    activeColor(activeColor),
    inactiveColor(inactiveColor),
    isActive(false),
    mouseDown(false)
  {
    this->SetText(text);
  }

  virtual void HandleEvent(const SDL_Event &e);
  virtual void Advance(float dt);
  virtual void Render(Renderer *renderer);
  virtual void Reset();
};

#endif /* _GRAVITY_BUTTON_WIDGET_HH_ */
