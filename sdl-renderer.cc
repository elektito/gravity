#include "sdl-renderer.hh"
#include "config.hh"

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
