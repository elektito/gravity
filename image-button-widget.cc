#include "image-button-widget.hh"
#include "screen.hh"
#include "helpers.hh"

ImageButtonWidget::ImageButtonWidget(Screen *screen,
                                     ResourceCache::Texture texture,
                                     float x,
                                     float y,
                                     float height,
                                     TextAnchor xanchor,
                                     TextAnchor yanchor,
                                     const SDL_Color &activeColor,
                                     const SDL_Color &inactiveColor) :
  ImageWidget(screen, texture, x, y, height, xanchor, yanchor),
  activeColorR((float) activeColor.r / 255),
  activeColorG((float) activeColor.g / 255),
  activeColorB((float) activeColor.b / 255),
  activeColorA((float) activeColor.a / 255),
  inactiveColorR((float) inactiveColor.r / 255),
  inactiveColorG((float) inactiveColor.g / 255),
  inactiveColorB((float) inactiveColor.b / 255),
  inactiveColorA((float) inactiveColor.a / 255),
  isActive(false),
  mouseDown(false)
{
}

void ImageButtonWidget::HandleEvent(const SDL_Event &e) {
  int mousex, mousey;
  float xp, yp;

  switch (e.type) {
  case SDL_MOUSEBUTTONDOWN:
    this->mouseDown = this->isActive;
    break;

  case SDL_MOUSEBUTTONUP:
    if (this->isActive && this->mouseDown)
      if (this->visible)
        this->screen->HandleWidgetEvent(BUTTON_CLICK, this);

    this->mouseDown = false;
    break;

  case SDL_MOUSEMOTION:
    SDL_GetMouseState(&mousex, &mousey);
    GetRelativeCoords(mousex, mousey, this->screen->window, this->xanchor, this->yanchor, xp, yp);

    bool xInRange = false;
    bool yInRange = false;

    // this->width is expressed as a ratio of screen
    // _height_. Calculate the width as a ratio of screen width for
    // the following calculations.
    int winw, winh;
    SDL_GetWindowSize(this->screen->window, &winw, &winh);
    float screenRatio = (float) winh / winw;
    float w = this->width * screenRatio;
    float nx = this->x * screenRatio;

    if (this->xanchor == TextAnchor::CENTER)
      xInRange = xp >= (nx - w / 2.0) && xp <= (nx + w / 2.0);
    else
      xInRange = xp >= nx && xp <= (nx + w);

    if (this->yanchor == TextAnchor::CENTER)
      yInRange = yp >= (this->y - this->height / 2.0) && yp <= (this->y + this->height / 2.0);
    else
      yInRange = yp >= this->y && yp <= (this->y + this->height);

    bool prevIsActive = this->isActive;
    this->isActive = xInRange && yInRange;

    if (this->isActive)
      this->SetColor(this->activeColorR, this->activeColorG, this->activeColorB, this->activeColorA);
    else
      this->SetColor(this->inactiveColorR, this->inactiveColorG, this->inactiveColorB, this->inactiveColorA);

    if (!prevIsActive && this->isActive)
      this->screen->HandleWidgetEvent(BUTTON_MOUSE_ENTER, this);
    else if (prevIsActive && !this->isActive)
      this->screen->HandleWidgetEvent(BUTTON_MOUSE_LEAVE, this);

    break;
  } // switch (e.type)

  this->ImageWidget::HandleEvent(e);
}

void ImageButtonWidget::Advance(float dt) {
  this->ImageWidget::Advance(dt);
}

void ImageButtonWidget::Render(Renderer *renderer) {
  this->ImageWidget::Render(renderer);
}

void ImageButtonWidget::Reset() {
  this->isActive = false;
  this->SetColor(this->inactiveColorR, this->inactiveColorG, this->inactiveColorB, this->inactiveColorA);
  this->mouseDown = false;
  this->ImageWidget::Reset();
}
