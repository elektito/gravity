#include "camera.hh"

void Camera::PointToScreen(SDL_Window *window, b2Vec2 p, int &x, int &y) {
  x = (p.x - this->pos.x) * this->ppm;
  y = (p.y - this->pos.y) * this->ppm;

  int w, h;
  SDL_GetWindowSize(window, &w, &h);
  y = h - y;
}

float32 Camera::LengthToScreen(float32 length) {
  return length * this->ppm;
}

b2Vec2 Camera::PointToWorld(int x, int y, SDL_Window *window) {
  int w, h;
  SDL_GetWindowSize(window, &w, &h);
  y = h - y;

  b2Vec2 p(x / ppm, y / ppm);
  p += b2Vec2(this->pos.x, this->pos.y);

  return p;
}

float32 Camera::LengthToWorld(float32 length) {
  return length / this->ppm;
}
