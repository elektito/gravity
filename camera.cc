#include "camera.hh"

Camera::Camera() :
  pos(0.0, 0.0),
  ppm(1.0)
{
}

void Camera::PointToScreen(SDL_Window *window, b2Vec2 p, int &x, int &y) const {
  x = (p.x - this->pos.x) * this->ppm;
  y = (p.y - this->pos.y) * this->ppm;

  int w, h;
  SDL_GetWindowSize(window, &w, &h);
  y = h - y;
}

float Camera::LengthToScreen(float length) const {
  return length * this->ppm;
}

b2Vec2 Camera::PointToWorld(int x, int y, SDL_Window *window) const {
  int w, h;
  SDL_GetWindowSize(window, &w, &h);
  y = h - y;

  b2Vec2 p(x / ppm, y / ppm);
  p += b2Vec2(this->pos.x, this->pos.y);

  return p;
}

float Camera::LengthToWorld(float length) const {
  return length / this->ppm;
}
