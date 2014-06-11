#include "sdl-renderer.hh"
#include "game-screen.hh"

#include <SDL2/SDL.h>

#include <iostream>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

const int TIME_STEP = 15;

using namespace std;

int main(int argc, char *argv[]) {
  bool quit = false;
  SDL_Window *window = nullptr;

  // Initialize SDL.
  if(SDL_Init(SDL_INIT_VIDEO) < 0) {
    cout << "SDL could not be initialized! SDL_Error: "
         << SDL_GetError() << endl;
    return 1;
  }

  // Create window.
  window = SDL_CreateWindow("polygons",
                            SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED,
                            SCREEN_WIDTH,
                            SCREEN_HEIGHT,
                            SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
  if (window == nullptr) {
    cout << "Window could not be created. SDL_Error: "
         << SDL_GetError() << endl;
    return 2;
  }

  Renderer *renderer = SdlRenderer::Create(window);
  Screen *currentScreen = new GameScreen(window);

  uint32_t lastTime = SDL_GetTicks();

  while (!quit) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      currentScreen->HandleEvent(e);

      if (e.type == SDL_QUIT)
        quit = true;
    }

    int dt = SDL_GetTicks() - lastTime;
    SDL_Delay(TIME_STEP > dt ? TIME_STEP - dt : 0);
    dt = SDL_GetTicks() - lastTime;
    currentScreen->Advance(dt / 1000.0);
    currentScreen->Render(renderer);
    lastTime = SDL_GetTicks();
  }

  delete renderer;

  // Destroy the window.
  SDL_DestroyWindow(window);

  // Quit SDL.
  SDL_Quit();

  return 0;
}
