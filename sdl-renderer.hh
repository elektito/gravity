#ifndef _GRAVITY_SDL_RENDERER_HH_
#define _GRAVITY_SDL_RENDERER_HH_

#include "renderer.hh"
#include "camera.hh"

#include <Box2D/Box2D.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <string>

using namespace std;

class SdlRenderer : public Renderer {
protected:
  SDL_Window *window;
  SDL_Renderer *renderer;
  TTF_Font *font;

  SdlRenderer(SDL_Window *window);

public:
  virtual ~SdlRenderer();

  virtual void ClearScreen(int r, int g, int b) const;
  virtual void DrawDisk(b2Vec2 pos, float32 radius, int r, int g, int b, int a) const;
  virtual void DrawPolygon(b2Vec2 vertices[], int count) const;
  virtual void DrawLine(b2Vec2 begin, b2Vec2 end, int r, int g, int b, int a) const;
  virtual void DrawTextM(string text, b2Vec2 pos, float32 height, SDL_Color c) const;
  virtual void DrawTextP(string text,
                         float32 x,
                         float32 y,
                         float32 height,
                         SDL_Color c,
                         TextAnchor xanchor,
                         TextAnchor yanchor) const;

  virtual void PresentScreen();

  static Renderer *Create(SDL_Window *window) {
    return new SdlRenderer(window);
  }
};

#endif /* _GRAVITY_SDL_RENDERER_HH_ */
