#ifndef _GRAVITY_RENDERER_HH_
#define _GRAVITY_RENDERER_HH_

#include "mesh.hh"
#include "camera.hh"

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

class Renderer {
protected:
  SDL_Window *window;
  SDL_GLContext context;
  Camera camera;
  GLuint backgroundTexture;
  int backgroundTextureWidth;
  int backgroundTextureHeight;
  GLuint backgroundVbo;

  int lastWindowWidth;
  int lastWindowHeight;

  void RebuildBackground();

public:
  Renderer(SDL_Window *window);
  virtual ~Renderer();

  void SetCamera(Camera &camera);
  void ClearScreen();
  void DrawBackground();
  void PresentScreen() const;
};

#endif /* _GRAVITY_RENDERER_HH_ */
