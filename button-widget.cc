#include "button-widget.hh"
#include "screen.hh"
#include "helpers.hh"

/// Converts the window coordinate (x, y) to relative coordinates (xp,
/// yp), i.e. xp and yp in range [0.0, 1.0], for a widget with the
/// given width and height in the given window.
void GetRelativeCoords(int x, int y,
                       SDL_Window *window,
                       TextAnchor xanchor, TextAnchor yanchor,
                       float32 &xp, float32 &yp)
{
  int winw, winh;
  SDL_GetWindowSize(window, &winw, &winh);

  if (xanchor == TextAnchor::LEFT)
    xp = (float32) x / winw;
  else if (xanchor == TextAnchor::RIGHT)
    xp = (float32) (winw - x) / winw;
  else if (xanchor == TextAnchor::CENTER)
    xp = (float32) (x - winw / 2) / winw;

  if (yanchor == TextAnchor::TOP)
    yp = (float32) y / winh;
  else if (yanchor == TextAnchor::BOTTOM)
    yp = (float32) (winh - y) / winh;
  else if (yanchor == TextAnchor::CENTER)
    yp = (float32) (winh / 2 - y) / winh;
}

void ButtonWidget::SetText(const string &text) {
  this->text = text;
  this->CalculateWidth();
}

string ButtonWidget::GetText() const {
  return this->text;
}

void ButtonWidget::CalculateWidth() {
  GetTextWidthP(text, this->height, this->screen->window, this->width);
}

void ButtonWidget::HandleEvent(const SDL_Event &e) {
  int x, y;
  float32 xp, yp;

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
    SDL_GetMouseState(&x, &y);
    GetRelativeCoords(x, y, this->screen->window, this->xanchor, this->yanchor, xp, yp);

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
  }
  else if (e.type == SDL_WINDOWEVENT) {
    if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
      this->CalculateWidth();
  }
}

void ButtonWidget::Advance(float dt) {

}

void ButtonWidget::Render(Renderer *renderer) {
  renderer->DrawTextP(this->text,
                      this->x, this->y, this->height,
                      this->isActive ? this->activeColor : this->inactiveColor,
                      this->xanchor, this->yanchor);
}
