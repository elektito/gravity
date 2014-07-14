#include "renderer.hh"
#include "resource-cache.hh"

#include <iostream>
#include <sstream>

using namespace std;

Renderer::Renderer(SDL_Window *window) :
  window(window)
{
  this->context = SDL_GL_CreateContext(window);

  // initialize glew
  GLenum status = glewInit();
  if (status != GLEW_OK) {
    cout << "Could not initialize GLEW." << endl;
    exit(1);
  }

  if (!GLEW_VERSION_3_3) {
    cout << "GLEW version 3.3 not found." << endl;
    exit(1);
  }

  // Enable blending.
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

Renderer::~Renderer() {

}

void Renderer::PresentScreen() const {
  SDL_GL_SwapWindow(this->window);
}

void Renderer::SetCamera(Camera &camera) {
  this->camera = camera;
}

void Renderer::ClearScreen() {
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
}
