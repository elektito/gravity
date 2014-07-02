#include "sdl-renderer.hh"
#include "config.hh"
#include "resource-cache.hh"

#include <SDL2/SDL2_gfxPrimitives.h>

#include <sstream>
#include <iomanip>
#include <exception>

using namespace std;

SdlRenderer::SdlRenderer(SDL_Window *window) :
  window(window)
{
  Uint32 flags = 0;
  if (Config::HardwareAcceleration)
    flags |= SDL_RENDERER_ACCELERATED;
  if (Config::VSync)
    flags |= SDL_RENDERER_PRESENTVSYNC;
  this->renderer = SDL_CreateRenderer(window, -1, flags);

  // Initialize resource cache.
  ResourceCache::Init(this->renderer);
}

SdlRenderer::~SdlRenderer() {
  ResourceCache::Finalize();
  SDL_DestroyRenderer(this->renderer);
}

void SdlRenderer::ClearScreen(int r, int g, int b) const {
  SDL_SetRenderDrawColor(this->renderer, r, g, b, 255);
  SDL_RenderClear(this->renderer);
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

void SdlRenderer::DrawTextM(string text, b2Vec2 pos, float32 height, SDL_Color color) const {
  int height_pixels = this->camera.LengthToScreen(height);

  TTF_Font *font = ResourceCache::GetFont(height_pixels);
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

  if (texture) {
    int x, y;
    this->camera.PointToScreen(this->window, pos, x, y);

    SDL_Rect dest {x, y, textSurface->w, textSurface->h};
    SDL_RenderCopy(this->renderer, texture, nullptr, &dest);
    SDL_DestroyTexture(texture);
  }

  SDL_FreeSurface(textSurface);
}

void SdlRenderer::DrawTextP(string text,
                            float32 x,
                            float32 y,
                            float32 height,
                            SDL_Color color,
                            TextAnchor xanchor,
                            TextAnchor yanchor) const
{
  int winw, winh;
  SDL_GetWindowSize(this->window, &winw, &winh);
  int height_pixels = height * winh;

  TTF_Font *font = ResourceCache::GetFont(height_pixels);
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

  if (texture) {
    int xp, yp;

    if (xanchor == TextAnchor::LEFT)
      xp = x * winw;
    else if (xanchor == TextAnchor::RIGHT)
      xp = winw - x * winw - textSurface->w;
    else if (xanchor == TextAnchor::CENTER)
      xp = winw / 2 + x * winw - textSurface->w / 2;

    if (yanchor == TextAnchor::TOP)
      yp = y * winh;
    else if (yanchor == TextAnchor::BOTTOM)
      yp = winh - y * winh - textSurface->h;
    else if (yanchor == TextAnchor::CENTER)
      yp = winh - (winh / 2 + y * winh + textSurface->h / 2);

    SDL_Rect dest {xp, yp, textSurface->w, textSurface->h};
    SDL_RenderCopy(this->renderer, texture, nullptr, &dest);
    SDL_DestroyTexture(texture);
  }

  SDL_FreeSurface(textSurface);
}

void SdlRenderer::DrawTexture(SDL_Texture *texture,
                              b2Vec2 bottomLeft,
                              float32 width,
                              float32 height) const
{
  int w = this->camera.LengthToScreen(width);
  int h = this->camera.LengthToScreen(height);
  int x, y;
  bottomLeft.y += height;
  this->camera.PointToScreen(this->window, bottomLeft, x, y);
  SDL_Rect dest {x, y, w, h};

  SDL_RenderCopy(this->renderer, texture, nullptr, &dest);
}

void SdlRenderer::DrawBackground(SDL_Texture *texture) const {
  int winw, winh;
  SDL_GetWindowSize(this->window, &winw, &winh);

  int texturew, textureh;
  SDL_QueryTexture(texture, nullptr, nullptr, &texturew, &textureh);

  // First try to fit the background texture to screen width.
  int w = winw;
  int h = w * ((float32) textureh / texturew);
  int x = 0;
  int y = (winh - h) / 2;

  if (h < winh) {
    // Won't do. Fit to screen height then.
    h = winh;
    w = h * ((float32) texturew / textureh);
    x = (winw - w) / 2;
    y = 0;
  }

  SDL_Rect dest {x, y, w, h};
  SDL_RenderCopy(this->renderer, texture, nullptr, &dest);
}
