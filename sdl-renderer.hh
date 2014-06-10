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

  void DrawDisk(b2Vec2 pos, float32 radius, int r, int g, int b, int a) const;

public:
  virtual ~SdlRenderer();

  virtual void DrawBackground() const;
  virtual void DrawGrid() const;
  virtual void DrawEntity(const Entity *entity) const;
  virtual void DrawTrail(const Entity *entity) const;
  virtual void DrawLine(b2Vec2 begin, b2Vec2 end, int r, int g, int b, int a) const;
  virtual void DrawText(string text,
                        SDL_Color color,
                        int scrx,
                        int scry,
                        bool anchorLeft=true,
                        bool anchorTop=true) const;
  virtual void PresentScreen();

  static Renderer *Create(SDL_Window *window) {
    return new SdlRenderer(window);
  }
};

#endif /* _GRAVITY_SDL_RENDERER_HH_ */
