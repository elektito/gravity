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
  Camera *camera;
  b2World *world;
  SDL_Renderer *renderer;
  TTF_Font *font;
  Trail *trail;

SdlRenderer(SDL_Window *window, Camera *camera, b2World *world, Trail *trail, Game *game);

  virtual void DrawBackground();
  virtual void DrawGrid();
  virtual void DrawEntities();
  virtual void DrawHud();
  virtual void PresentScreen();

  void DrawDisk(b2Vec2 pos, float32 radius, int r, int g, int b, int a);
  void DrawLine(b2Vec2 begin, b2Vec2 end, int r, int g, int b, int a);
  void DrawText(string text,
                SDL_Color color,
                int scrx,
                int scry,
                bool anchorLeft=true,
                bool anchorTop=true);
  void DrawScore();
  void DrawTime();
  void DrawTrail(Trail *t);

public:
  virtual ~SdlRenderer();

  static Renderer *Create(SDL_Window *window,
                          Camera *camera,
                          b2World *world,
                          Trail *trail,
                          Game *game)
  {
return new SdlRenderer(window, camera, world, trail, game);
  }
};

#endif /* _GRAVITY_SDL_RENDERER_HH_ */
