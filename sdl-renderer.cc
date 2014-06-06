#include "sdl-renderer.hh"

#include <SDL2/SDL2_gfxPrimitives.h>

#include <sstream>
#include <iomanip>
#include <exception>

using namespace std;

SdlRenderer::SdlRenderer(SDL_Window *window,
                         Camera *camera,
                         b2World *world,
                         Trail *trail,
                         Game *game) :
  Renderer(game),
  window(window),
  camera(camera),
  world(world),
  trail(trail)
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

void SdlRenderer::DrawBackground() {
  SDL_SetRenderDrawColor(this->renderer, 0, 0, 255, 255);
  SDL_RenderClear(this->renderer);
}

void SdlRenderer::DrawGrid() {
  // Draw grid.
  int winw, winh;
  SDL_GetWindowSize(this->window, &winw, &winh);

  float32 upperx = camera->pos.x + winw / camera->ppm;
  float32 uppery = camera->pos.y + winh / camera->ppm;
  float32 x = camera->pos.x + fmod(camera->pos.x + 10, 10);
  float32 y = camera->pos.x + fmod(camera->pos.y + 10, 10);
  for (; x <= upperx; x += 10)
    this->DrawLine(b2Vec2(x, camera->pos.y), b2Vec2(x, uppery), 32, 64, 64, 255);
  for (; y <= uppery; y += 10)
    this->DrawLine(b2Vec2(camera->pos.x, y), b2Vec2(upperx, y), 32, 64, 64, 255);
}

void SdlRenderer::DrawEntities() {
  this->DrawTrail(this->trail);

  for (b2Body *b = this->world->GetBodyList(); b; b = b->GetNext()) {
    for (b2Fixture *f = b->GetFixtureList(); f; f = f->GetNext()) {
      this->DrawDisk(b->GetPosition(), f->GetShape()->m_radius, 255, 255, 255, 255);
    }
  }
}

void SdlRenderer::DrawHud() {
  this->DrawLine(b2Vec2(0, 1), b2Vec2(0, -1), 255, 0, 0, 255);
  this->DrawLine(b2Vec2(1, 0), b2Vec2(-1, 0), 255, 0, 0, 255);

  this->DrawScore();
  this->DrawTime();
}

void SdlRenderer::PresentScreen() {
  SDL_RenderPresent(this->renderer);
}

void SdlRenderer::DrawDisk(b2Vec2 pos, float32 radius, int r, int g, int b, int a) {
  int x, y;
  radius = this->camera->LengthToScreen(radius);
  this->camera->PointToScreen(this->window, pos, x, y);

  aacircleRGBA(this->renderer, x, y, radius, r, g, b, a);
  filledCircleRGBA(this->renderer, x, y, radius, r, g, b, a);
}

void SdlRenderer::DrawLine(b2Vec2 begin, b2Vec2 end, int r, int g, int b, int a) {
  int x1, y1, x2, y2;
  this->camera->PointToScreen(this->window, begin, x1, y1);
  this->camera->PointToScreen(this->window, end, x2, y2);

  SDL_SetRenderDrawColor(this->renderer, r, g, b, a);
  SDL_RenderDrawLine(this->renderer, x1, y1, x2, y2);
}

void SdlRenderer::DrawText(string text,
                           SDL_Color color,
                           int scrx,
                           int scry,
                           bool anchorLeft,
                           bool anchorTop) {
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

void SdlRenderer::DrawScore() {
  stringstream ss;
  ss << setw(6) << setfill('0') << this->game->score;
  this->DrawText(ss.str(), SDL_Color {0, 0, 0, 128}, 10, 10, false, true);
}

void SdlRenderer::DrawTime() {
  int minutes = this->game->timeRemaining / 60;
  int seconds = this->game->timeRemaining % 60;
  stringstream ss;
  ss << setw(2) << setfill('0') << minutes
     << setw(0) << ":"
     << setw(2) << setfill('0') << seconds;
  this->DrawText(ss.str(), SDL_Color {0, 0, 0, 128}, 10, 10);
}

void SdlRenderer::DrawTrail(Trail *t) {
  vector<TrailPoint> points;

  if (t->size < t->points.size()) {
    // Choose 't->size' points in the 't->time' time-window.

    // From the rest choose enough, as evenly timed as possible.
    auto step = t->time / t->size;
    auto time = t->points[0].time;
    auto it = t->points.begin();
    while (points.size() < t->size) {
      time += step;

      // Go forward among previous locations until we reach one after
      // 'time'. Choose the point as close to the time we want as
      // possible.
      float32 leastDiff = FLT_MAX;
      TrailPoint closestPoint;
      for (; it != t->points.end(); ++it) {
        if (abs(it->time - time) < leastDiff) {
          leastDiff = abs(it->time - time);
          closestPoint = *it;
        }

        if (it->time >= time)
          break;
      }

      points.push_back(closestPoint);

      // Continue from this point.
      time = closestPoint.time;
    }
  }
  else
    points = t->points;

  float32 r = t->body->GetFixtureList()->GetShape()->m_radius;
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
