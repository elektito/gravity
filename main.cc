#include "sdl-renderer.hh"
#include "game-screen.hh"
#include "high-scores-screen.hh"
#include "main-menu-screen.hh"

#include <SDL2/SDL.h>

#include <iostream>
#include <fstream>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

const int TIME_STEP = 5;

using namespace std;

int main(int argc, char *argv[]) {
  bool quit = false;
  SDL_Window *window = nullptr;

  // Seed the pseudo-random number generator with time.
  srand(time(0));

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

  Screen *mainMenuScreen = new MainMenuScreen(window);
  Screen *gameScreen = new GameScreen(window);
  Screen *highScoresScreen = new HighScoresScreen(window);

  ifstream input("gravity.save", ifstream::in | ifstream::binary);
  if (input) {
    highScoresScreen->Load(input);
  }
  else
    cout << "No save file." << endl;
  input.close();

  Screen *currentScreen = mainMenuScreen;

  uint32_t lastTime = SDL_GetTicks();

  while (!quit) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      currentScreen->HandleEvent(e);

      if (e.type == SDL_QUIT)
        quit = true;
      else if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
        case SDLK_q:
          SDL_Event quitEvent;
          quitEvent.type = SDL_QUIT;
          SDL_PushEvent(&quitEvent);
          break;
        case SDLK_f:
          auto flags = SDL_GetWindowFlags(window);
          if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP)
            SDL_SetWindowFullscreen(window, SDL_FALSE);
          else
            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
          break;
        }
      }
    }

    int dt = SDL_GetTicks() - lastTime;
    SDL_Delay(TIME_STEP > dt ? TIME_STEP - dt : 0);
    dt = SDL_GetTicks() - lastTime;
    lastTime = SDL_GetTicks();
    currentScreen->Advance(dt / 1000.0);
    currentScreen->Render(renderer);

    if (currentScreen->state["name"] == "game-over") {
      highScoresScreen->SwitchScreen(currentScreen->state);
      currentScreen = highScoresScreen;
    }
    else if (currentScreen->state["name"] == "menu-new-game-selected") {
      gameScreen->Reset();
      gameScreen->SwitchScreen(currentScreen->state);
      currentScreen = gameScreen;
    }
    else if (currentScreen->state["name"] == "menu-highscores-selected") {
      highScoresScreen->SwitchScreen(currentScreen->state);
      currentScreen = highScoresScreen;
    }
    else if (currentScreen->state["name"] == "menu-exit-selected") {
      SDL_Event quitEvent;
      quitEvent.type = SDL_QUIT;
      SDL_PushEvent(&quitEvent);
      break;
    }
    else if (currentScreen->state["name"] == "highscores-manu-selected") {
      mainMenuScreen->SwitchScreen(currentScreen->state);
      currentScreen = mainMenuScreen;
    }
  }

  ofstream output("gravity.save", ofstream::out | ofstream::binary);
  if (output) {
    highScoresScreen->Save(output);
  }
  else {
    cout << "Could not write to save file." << endl;
  }

  delete renderer;

  // Destroy the window.
  SDL_DestroyWindow(window);

  // Quit SDL.
  SDL_Quit();

  return 0;
}
