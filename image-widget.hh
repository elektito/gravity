#ifndef _GRAVITY_IMAGE_WIDGET_HH_
#define _GRAVITY_IMAGE_WIDGET_HH_

#include "widget.hh"
#include "resource-cache.hh"

using namespace std;

class ImageWidget : public Widget {
protected:
  float x;
  float y;
  float height;
  float width;
  TextAnchor xanchor;
  TextAnchor yanchor;
  ResourceCache::Texture texture;
  GLuint vbo;
  struct {
    float r;
    float g;
    float b;
    float a;
  } color;

public:
  ImageWidget(Screen *screen, ResourceCache::Texture texture, float x, float y, float height, TextAnchor xanchor, TextAnchor yanchor, const SDL_Color &color={255, 255, 255, 255});
  virtual ~ImageWidget();

  void SetColor(float r, float g, float b, float a);
  void SetTexture(ResourceCache::Texture texture);

  virtual void HandleEvent(const SDL_Event &e);
  virtual void Advance(float dt);
  virtual void Render(Renderer *renderer);
  virtual void Reset();
};

#endif /* _GRAVITY_IMAGE_WIDGET_HH_ */
