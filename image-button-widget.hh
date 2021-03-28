#ifndef _GRAVITY_IMAGE_BUTTON_WIDGET_HH_
#define _GRAVITY_IMAGE_BUTTON_WIDGET_HH_

#include "image-widget.hh"
#include "resource-cache.hh"

#define BUTTON_CLICK 1000
#define BUTTON_MOUSE_ENTER 1001
#define BUTTON_MOUSE_LEAVE 1002

class ImageButtonWidget : public ImageWidget {
protected:
  float activeColorR;
  float activeColorG;
  float activeColorB;
  float activeColorA;
  float inactiveColorR;
  float inactiveColorG;
  float inactiveColorB;
  float inactiveColorA;

  bool isActive;
  bool mouseDown;

public:
  ImageButtonWidget(Screen *screen,
                    ResourceCache::Texture texture,
                    float x,
                    float y,
                    float height,
                    TextAnchor xanchor,
                    TextAnchor yanchor,
                    const SDL_Color &activeColor,
                    const SDL_Color &inactiveColor);

  virtual void HandleEvent(const SDL_Event &e);
  virtual void Advance(float dt);
  virtual void Render(Renderer *renderer);
  virtual void Reset();
};

#endif /* _GRAVITY_IMAGE_BUTTON_WIDGET_HH_ */
