#ifndef _GRAVITY_RENDERER_HH_
#define _GRAVITY_RENDERER_HH_

#include "mesh.hh"
#include "camera.hh"
#include "resource-cache.hh"

#include <SDL2/SDL.h>

#include <string>

using namespace std;

enum class TextAnchor {
  LEFT,
  RIGHT,
  CENTER,
  TOP,
  BOTTOM
};

class Background {
protected:
  SDL_Window *window;
  ResourceCache::Texture texture;
  GLuint vbo;

  int lastWindowWidth;
  int lastWindowHeight;

  void RebuildIfNecessary();

public:
  Background(SDL_Window *window, ResourceCache::Texture texture);
  virtual ~Background();

  void Draw();
};

class Renderer {
protected:
  SDL_Window *window;
  SDL_GLContext context;
  Camera camera;

public:
  Renderer(SDL_Window *window);
  virtual ~Renderer();

  void SetCamera(Camera &camera);
  void ClearScreen();
  void PresentScreen() const;
};

#endif /* _GRAVITY_RENDERER_HH_ */
