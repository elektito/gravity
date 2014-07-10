#ifndef _GRAVITY_LABEL_WIDGET_HH_
#define _GRAVITY_LABEL_WIDGET_HH_

#include "widget.hh"

#include <SDL2/SDL.h>

#include <string>

using namespace std;

class LabelWidget : public Widget {
protected:
  string text;
  float x;
  float y;
  float height;
  TextAnchor xanchor;
  TextAnchor yanchor;
  SDL_Color color;

  float width;

  GLuint texture;
  GLuint vbo;

  void Rebuild();

public:
  LabelWidget(Screen *screen, const string &text, float x, float y, float height, TextAnchor xanchor, TextAnchor yanchor, const SDL_Color &color) :
    Widget(screen),
    text(text),
    x(x),
    y(y),
    height(height),
    xanchor(xanchor),
    yanchor(yanchor),
    color(color),
    texture(0)
  {
    this->Reset();
  }

  virtual ~LabelWidget();

  void SetText(const string &text);
  const string &GetText() const;
  void SetColor(const SDL_Color &c);
  const SDL_Color &GetColor() const;

  virtual void HandleEvent(const SDL_Event &e);
  virtual void Advance(float dt);
  virtual void Render(Renderer *renderer);
  virtual void Reset();
};

#endif /* _GRAVITY_LABEL_WIDGET_HH_ */
