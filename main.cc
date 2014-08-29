#include "renderer.hh"
#include "credits-screen.hh"
#include "splash-screen.hh"
#include "game-screen.hh"
#include "high-scores-screen.hh"
#include "main-menu-screen.hh"
#include "resource-cache.hh"
#include "config.hh"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <SDL2/SDL.h>

#include <iostream>
#include <fstream>

using namespace std;

void HandleEvents(SDL_Event &e, SDL_Window *window, bool &quit) {
  uint32_t flags;
  int winw, winh;

  switch (e.type) {
  case SDL_QUIT:
    quit = true;
    break;

  case SDL_KEYDOWN:
    switch (e.key.keysym.sym) {
    case SDLK_q:
      SDL_Event quitEvent;
      quitEvent.type = SDL_QUIT;
      SDL_PushEvent(&quitEvent);
      break;

    case SDLK_f:
      flags = SDL_GetWindowFlags(window);
      if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP)
        SDL_SetWindowFullscreen(window, SDL_FALSE);
      else
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
      break;

    case SDLK_s:
      SDL_GetWindowSize(window, &winw, &winh);
      GLubyte *pixels = new GLubyte[4 * winw * winh];
      glReadPixels(0, 0, winw, winh, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

      // Flip the image so it will be the right way up in the image
      // file.
      auto rowsize = winw * 4;
      GLubyte *tmprow = new GLubyte[rowsize];
      for (int y = 0; y < winh / 2; y++) {
        auto row1 = pixels + y * rowsize;
        auto row2 = pixels + (winh-1-y) * rowsize;
        memcpy(tmprow, row1, rowsize);
        memcpy(row1, row2, rowsize);
        memcpy(row2, tmprow, rowsize);
      }
      delete[] tmprow;

      string filename = tmpnam(nullptr);
      filename += ".png";
      stbi_write_png(filename.data(), winw, winh, 4, pixels, 0);
      cout << "Saved screenshot to " << filename << endl;

      delete[] pixels;
      break;
    }
    break;

  case SDL_WINDOWEVENT:
    if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
      int winw, winh;
      SDL_GetWindowSize(window, &winw, &winh);

      // Update OpenGL viewport.
      glViewport(0, 0, winw, winh);

      // Update window size in shader.
      auto program = ResourceCache::texturedPolygonProgram;
      glUseProgram(program);
      GLuint resolutionUniform = glGetUniformLocation(program, "resolution");
      glUniform2f(resolutionUniform, winw, winh);
      glUseProgram(0);

      program = ResourceCache::hudTexturedPolygonProgram;
      glUseProgram(program);
      resolutionUniform = glGetUniformLocation(program, "resolution");
      glUniform2f(resolutionUniform, winw, winh);
      glUseProgram(0);
    }
    break;
  } // switch (e.type)
}

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
                            Config::ScreenWidth,
                            Config::ScreenHeight,
                            SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
  if (window == nullptr) {
    cout << "Window could not be created. SDL_Error: "
         << SDL_GetError() << endl;
    return 2;
  }

  Renderer *renderer = new Renderer(window);
  ResourceCache::Init();

  Screen *splashScreen = new SplashScreen(window);
  SDL_ShowWindow(window);

  // Set resolution uniforms in shader programs that need it.
  int winw, winh;
  SDL_GetWindowSize(window, &winw, &winh);
  auto program = ResourceCache::texturedPolygonProgram;
  glUseProgram(program);
  GLuint resolutionUniform = glGetUniformLocation(program, "resolution");
  glUniform2f(resolutionUniform, winw, winh);
  glUseProgram(0);

  program = ResourceCache::hudTexturedPolygonProgram;
  glUseProgram(program);
  resolutionUniform = glGetUniformLocation(program, "resolution");
  glUniform2f(resolutionUniform, winw, winh);
  glUseProgram(0);

  // On some systems (like on StumpWM), a size change might happen
  // right after the window is shown. This takes care of that.
  SDL_Event e;
  while (SDL_PollEvent(&e))
    HandleEvents(e, window, quit);

  splashScreen->Render(renderer);

  Screen *mainMenuScreen = new MainMenuScreen(window);
  splashScreen->Render(renderer);

  Screen *gameScreen = new GameScreen(window);
  splashScreen->Render(renderer);

  Screen *highScoresScreen = new HighScoresScreen(window);
  splashScreen->Render(renderer);

  Screen *creditsScreen = new CreditsScreen(window);
  splashScreen->Render(renderer);

#ifdef RELEASE_BUILD
  SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
#endif

  ifstream input("gravity.save", ifstream::in | ifstream::binary);
  if (input) {
    highScoresScreen->Load(input);
  }
  else
    cout << "No save file." << endl;
  input.close();

  Screen *currentScreen = splashScreen;

  uint32_t lastTime = SDL_GetTicks();

  while (!quit) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      currentScreen->HandleEvent(e);
      HandleEvents(e, window, quit);
    } // while (SDL_PollEvent(&e))

    int dt = SDL_GetTicks() - lastTime;
    SDL_Delay(Config::TimeStep > dt ? Config::TimeStep - dt : 0);
    dt = SDL_GetTicks() - lastTime;
    lastTime = SDL_GetTicks();
    currentScreen->Advance(dt / 1000.0);
    currentScreen->Render(renderer);

    if (currentScreen->state["name"] == "splash-over") {
      mainMenuScreen->SwitchScreen(currentScreen->state);
      currentScreen = mainMenuScreen;
    }
    else if (currentScreen->state["name"] == "game-over") {
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
    else if (currentScreen->state["name"] == "menu-credits-selected") {
      creditsScreen->SwitchScreen(currentScreen->state);
      currentScreen = creditsScreen;
    }
    else if (currentScreen->state["name"] == "highscores-manu-selected") {
      mainMenuScreen->SwitchScreen(currentScreen->state);
      currentScreen = mainMenuScreen;
    }
    else if (currentScreen->state["name"] == "credits-manu-selected") {
      mainMenuScreen->SwitchScreen(currentScreen->state);
      currentScreen = mainMenuScreen;
    }
  } // while (!quit)

  ofstream output("gravity.save", ofstream::out | ofstream::binary);
  if (output) {
    highScoresScreen->Save(output);
  }
  else {
    cout << "Could not write to save file." << endl;
  }

  delete mainMenuScreen;
  delete highScoresScreen;
  delete gameScreen;

  delete renderer;

  // Destroy the window.
  SDL_DestroyWindow(window);

  // Quit SDL.
  SDL_Quit();

  return 0;
}
