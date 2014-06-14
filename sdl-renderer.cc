#include "sdl-renderer.hh"

#include <SDL2/SDL2_gfxPrimitives.h>

#include <sstream>
#include <iomanip>
#include <exception>

using namespace std;

b2Vec2 RotatePoint(b2Vec2 point, float32 theta, b2Vec2 center) {
  b2Vec2 np;
  point = point - center;
  np.x = point.x * cos(theta) - point.y * sin(theta);
  np.y = point.x * sin(theta) + point.y * cos(theta);
  return np + center;
}

SdlRenderer::SdlRenderer(SDL_Window *window) :
  window(window)
{
  this->renderer = SDL_CreateRenderer(window, -1,
                                      SDL_RENDERER_ACCELERATED |
                                      SDL_RENDERER_PRESENTVSYNC);

  // Initialize fonts.
  if (TTF_Init() == -1) {
    stringstream ss;
    ss << "Could not initialize SDL_ttf. SDL_ttf error: "
       << TTF_GetError();
    throw runtime_error(ss.str());
  }

  // Load fonts.
  this->font = TTF_OpenFont("fonts/UbuntuMono-B.ttf", 72);
  if (font == nullptr) {
    stringstream ss;
    ss << "Unable to load font. SDL_ttf error: " << TTF_GetError();
    throw runtime_error(ss.str());
  }
}

SdlRenderer::~SdlRenderer() {
  TTF_CloseFont(this->font);
  TTF_Quit();

  SDL_DestroyRenderer(this->renderer);
}

void SdlRenderer::DrawBackground() const {
  SDL_SetRenderDrawColor(this->renderer, 0, 0, 255, 255);
  SDL_RenderClear(this->renderer);
}

void SdlRenderer::DrawGrid() const {
  // Draw grid.
  int winw, winh;
  SDL_GetWindowSize(this->window, &winw, &winh);

  float32 upperx = this->camera.pos.x + winw / this->camera.ppm;
  float32 uppery = this->camera.pos.y + winh / this->camera.ppm;
  float32 x = this->camera.pos.x + fmod(this->camera.pos.x + 10, 10);
  float32 y = this->camera.pos.x + fmod(this->camera.pos.y + 10, 10);
  for (; x <= upperx; x += 10)
    this->DrawLine(b2Vec2(x, this->camera.pos.y), b2Vec2(x, uppery), 32, 64, 64, 255);
  for (; y <= uppery; y += 10)
    this->DrawLine(b2Vec2(this->camera.pos.x, y), b2Vec2(upperx, y), 32, 64, 64, 255);
}

void SdlRenderer::DrawEntity(const Entity *entity) const {
  b2Body *b = entity->body;
  for (b2Fixture *f = b->GetFixtureList(); f; f = f->GetNext()) {
    b2Shape *shape = f->GetShape();
    if (shape->GetType() == b2Shape::e_circle)
      this->DrawDisk(b->GetPosition(), shape->m_radius, 255, 255, 255, 255);
    else if (shape->GetType() == b2Shape::e_polygon) {
      b2Vec2 pos = b->GetPosition();
      float32 angle = b->GetAngle();
      b2PolygonShape *polygon = (b2PolygonShape*) shape;
      int count = polygon->GetVertexCount();
      b2Vec2 vertices[count];
      for (int i = 0; i < count; ++i) {
        vertices[i] = pos + polygon->m_vertices[i];
        vertices[i] = RotatePoint(vertices[i], angle, pos);
      }

      this->DrawPolygon(vertices, count);
    }
  }
}

void SdlRenderer::PresentScreen() {
  SDL_RenderPresent(this->renderer);
}

void SdlRenderer::DrawDisk(b2Vec2 pos, float32 radius, int r, int g, int b, int a) const {
  int x, y;
  radius = this->camera.LengthToScreen(radius);
  this->camera.PointToScreen(this->window, pos, x, y);

  aacircleRGBA(this->renderer, x, y, radius, r, g, b, a);
  filledCircleRGBA(this->renderer, x, y, radius, r, g, b, a);
}

void SdlRenderer::DrawPolygon(b2Vec2 vertices[], int count) const {
  Sint16 vx[count];
  Sint16 vy[count];
  int x, y;
  for (int i = 0; i < count; ++i) {
    this->camera.PointToScreen(this->window, vertices[i], x, y);
    vx[i] = x;
    vy[i] = y;
  }

  filledPolygonRGBA(this->renderer, vx, vy, count, 255, 255, 255, 255);
}

void SdlRenderer::DrawLine(b2Vec2 begin, b2Vec2 end, int r, int g, int b, int a) const {
  int x1, y1, x2, y2;
  this->camera.PointToScreen(this->window, begin, x1, y1);
  this->camera.PointToScreen(this->window, end, x2, y2);

  SDL_SetRenderDrawColor(this->renderer, r, g, b, a);
  SDL_RenderDrawLine(this->renderer, x1, y1, x2, y2);
}

void SdlRenderer::DrawText(string text,
                           SDL_Color color,
                           int scrx,
                           int scry,
                           bool anchorLeft,
                           bool anchorTop) const
{
  SDL_Surface *textSurface = TTF_RenderText_Solid(font, text.data(), color);
  if (textSurface == nullptr) {
    stringstream ss;
    ss << "Unable to render text surface. SDL_ttf error: "
       << TTF_GetError();
    throw runtime_error(ss.str());
  }

  auto texture = SDL_CreateTextureFromSurface(this->renderer, textSurface);
  if (texture == nullptr) {
    stringstream ss;
    ss << "Unable to create texture from rendered text. SDL error: "
       << SDL_GetError();
    throw runtime_error(ss.str());
  }

  SDL_FreeSurface(textSurface);

  if (texture) {
    int winw, winh;
    SDL_GetWindowSize(this->window, &winw, &winh);
    int x, y;
    x = anchorLeft ? scrx : winw - textSurface->w - scrx;
    y = anchorTop ? scry : winh - textSurface->h - scry;
    SDL_Rect dest = {x, y, textSurface->w, textSurface->h};
    SDL_RenderCopy(this->renderer, texture, nullptr, &dest);
    SDL_DestroyTexture(texture);
  }
}

void SdlRenderer::DrawTrail(const Entity *e) const {
  vector<TrailPoint> points;

  if (!e->hasTrail)
    return;

  if (e->trail.size < e->trail.points.size()) {
    // Choose 'e->trail.size' points in the 'e->trail.time' time-window.

    // From the rest choose enough, as evenly timed as possible.
    auto step = e->trail.time / e->trail.size;
    auto time = e->trail.points.back().time;
    auto it = e->trail.points.rbegin();
    while (points.size() < e->trail.size) {
      time -= step;

      // Go forward among previous locations until we reach one after
      // 'time'. Choose the point as close to the time we want as
      // possible.
      float32 leastDiff = FLT_MAX;
      TrailPoint closestPoint;
      for (; it != e->trail.points.rend(); ++it) {
        if (abs(it->time - time) < leastDiff) {
          leastDiff = abs(it->time - time);
          closestPoint = *it;
        }

        if (it->time <= time)
          break;
      }

      points.push_back(closestPoint);

      // Continue from this point.
      time = closestPoint.time;
    }

    // We have chosen the trail points from the last to the first, so
    // reverse them.
    std::reverse(points.begin(), points.end());
  }
  else
    points = e->trail.points;

  float32 r = e->body->GetFixtureList()->GetShape()->m_radius;
  auto startr = r / 10.0;
  auto endr = r / 2.0;
  r = startr;
  float32 a = 128;
  float32 dr, da;
  if (points.size() > 0) {
    dr = (endr - startr) / points.size();
    da = (255 - a) / points.size();
  }

  for (auto &p : points) {
    this->DrawDisk(p.pos, r, 255, 0, 0, a);
    r += dr;
    a += da;
  }
}
