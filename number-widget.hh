#ifndef _GRAVITY_NUMBER_WIDGET_HH_
#define _GRAVITY_NUMBER_WIDGET_HH_

#include "widget.hh"

class NumberWidget : public Widget {
protected:
  float x;
  float y;
  float height;
  float width;
  TextAnchor xanchor;
  TextAnchor yanchor;
  GLuint vbo;
  uint32_t ndigits;
  struct {
    float r;
    float g;
    float b;
    float a;
  } color;

public:
  NumberWidget(Screen *screen, uint32_t n, float x, float y, float height, TextAnchor xanchor, TextAnchor yanchor, uint32_t ndigits, const SDL_Color &color={255, 255, 255, 255});
  virtual ~NumberWidget();

  void SetColor(float r, float g, float b, float a);
  void SetColor(const SDL_Color &c);
  void SetNumber(uint32_t n);

  virtual void HandleEvent(const SDL_Event &e);
  virtual void Advance(float dt);
  virtual void Render(Renderer *renderer);
  virtual void Reset();
};

#endif /* _GRAVITY_NUMBER_WIDGET_HH_ */
