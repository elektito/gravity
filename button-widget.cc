#include "button-widget.hh"
#include "screen.hh"
#include "helpers.hh"

void ButtonWidget::HandleEvent(const SDL_Event &e) {
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

    if (this->xanchor == TextAnchor::CENTER)
      xInRange = xp >= (this->x - this->width / 2.0) && xp <= (this->x + this->width / 2.0);
    else
      xInRange = xp >= this->x && xp <= (this->x + this->width);

    if (this->yanchor == TextAnchor::CENTER)
      yInRange = yp >= (this->y - this->height / 2.0) && yp <= (this->y + this->height / 2.0);
    else
      yInRange = yp >= this->y && yp <= (this->y + this->height);

    bool prevIsActive = this->isActive;
    this->isActive = xInRange && yInRange;

    if (this->isActive)
      this->SetColor(this->activeColor);
    else
      this->SetColor(this->inactiveColor);

    if (!prevIsActive && this->isActive)
      this->screen->HandleWidgetEvent(BUTTON_MOUSE_ENTER, this);
    else if (prevIsActive && !this->isActive)
      this->screen->HandleWidgetEvent(BUTTON_MOUSE_LEAVE, this);

    break;
  } // switch (e.type)

  this->LabelWidget::HandleEvent(e);
}

void ButtonWidget::Advance(float dt) {
  this->LabelWidget::Advance(dt);
}

void ButtonWidget::Render(Renderer *renderer) {
  this->LabelWidget::Render(renderer);
}

void ButtonWidget::Reset() {
  this->isActive = false;
  this->SetColor(this->inactiveColor);
  this->mouseDown = false;
  this->LabelWidget::Reset();
}
