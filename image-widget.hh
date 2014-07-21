#ifndef _GRAVITY_IMAGE_WIDGET_HH_
#define _GRAVITY_IMAGE_WIDGET_HH_

#include "widget.hh"

using namespace std;

class ImageWidget : public Widget {
protected:
  float x;
  float y;
  float height;
  float width;
  TextAnchor xanchor;
  TextAnchor yanchor;
  GLuint texture;
  GLuint vbo;

public:
  ImageWidget(Screen *screen, GLuint texture, float x, float y, float height, TextAnchor xanchor, TextAnchor yanchor);
  virtual ~ImageWidget();

  virtual void HandleEvent(const SDL_Event &e);
  virtual void Advance(float dt);
  virtual void Render(Renderer *renderer);
  virtual void Reset();
};

#endif /* _GRAVITY_IMAGE_WIDGET_HH_ */
