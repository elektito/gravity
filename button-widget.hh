#ifndef _GRAVITY_BUTTON_WIDGET_HH_
#define _GRAVITY_BUTTON_WIDGET_HH_

#include "widget.hh"

#include <string>

using namespace std;

class ButtonWidget : public Widget {
protected:
  string text;
  float32 x;
  float32 y;
  float32 height;
  TextAnchor xanchor;
  TextAnchor yanchor;
  SDL_Color activeColor;
  SDL_Color inactiveColor;

  bool isActive;
  float32 width;

  void CalculateWidth();

public:
  ButtonWidget(Screen *screen,
               string text,
               float32 x,
               float32 y,
               float32 height,
               TextAnchor xanchor,
               TextAnchor yanchor,
               const SDL_Color &activeColor,
               const SDL_Color &inactiveColor) :
    Widget(screen),
    x(x),
    y(y),
    height(height),
    xanchor(xanchor),
    yanchor(yanchor),
    activeColor(activeColor),
    inactiveColor(inactiveColor),
    isActive(false)
  {
    this->SetText(text);
  }

  void SetText(const string &text);
  string GetText() const;

  virtual void HandleEvent(const SDL_Event &e);
  virtual void Advance(float dt);
  virtual void Render(Renderer *renderer);
};

#endif /* _GRAVITY_BUTTON_WIDGET_HH_ */
