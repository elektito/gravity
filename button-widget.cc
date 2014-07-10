#include "button-widget.hh"
#include "screen.hh"
#include "helpers.hh"

/// Converts the window coordinate (x, y) to relative coordinates (xp,
/// yp), i.e. xp and yp in range [0.0, 1.0], for a widget with the
/// given width and height in the given window.
void GetRelativeCoords(int x, int y,
                       SDL_Window *window,
                       TextAnchor xanchor, TextAnchor yanchor,
                       float &xp, float &yp)
{
  int winw, winh;
  SDL_GetWindowSize(window, &winw, &winh);

  if (xanchor == TextAnchor::LEFT)
    xp = (float) x / winw;
  else if (xanchor == TextAnchor::RIGHT)
    xp = (float) (winw - x) / winw;
  else if (xanchor == TextAnchor::CENTER)
    xp = (float) (x - winw / 2) / winw;

  if (yanchor == TextAnchor::TOP)
    yp = (float) y / winh;
  else if (yanchor == TextAnchor::BOTTOM)
    yp = (float) (winh - y) / winh;
  else if (yanchor == TextAnchor::CENTER)
    yp = (float) (winh / 2 - y) / winh;
}

void ButtonWidget::HandleEvent(const SDL_Event &e) {
  int mousex, mousey;
  float xp, yp;

  if (e.type == SDL_MOUSEBUTTONDOWN) {
    this->mouseDown = this->isActive;
  }
  else if (e.type == SDL_MOUSEBUTTONUP) {
    if (this->isActive && this->mouseDown) {
      this->screen->HandleWidgetEvent(BUTTON_CLICK, this);
    }

    this->mouseDown = false;
  }
  else if (e.type == SDL_MOUSEMOTION) {
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

    this->isActive = xInRange && yInRange;
    if (this->isActive)
      this->SetColor(this->activeColor);
    else
      this->SetColor(this->inactiveColor);
  }

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
  this->mouseDown = false;
  this->LabelWidget::Reset();
}
