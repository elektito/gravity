#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <Box2D/Box2D.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

const int TIME_STEP = 15;

const float32 ppm = 5.0;
const float32 camerax = -50.0;
const float32 cameray = -50.0;

using namespace std;

b2Vec2 WindowToWorldCoords(int x, int y, SDL_Window *window) {
  int w, h;
  SDL_GetWindowSize(window, &w, &h);
  y = h - y;

  b2Vec2 p(x / ppm, y / ppm);
  p += b2Vec2(camerax, cameray);

  return p;
}

b2Body *GetBodyFromPoint(b2Vec2 p, b2World *world) {
  for (b2Body *b = world->GetBodyList(); b; b = b->GetNext()) {
    for (b2Fixture *f = b->GetFixtureList(); f; f = f->GetNext()) {
      if (f->TestPoint(p))
        return b;
    }
  }

  return nullptr;
}

struct Entity {
  b2Body *body;
  bool isGravitySource;
  float32 gravityCoeff;
};

class Renderer {
protected:
  SDL_Window *window;
  b2World *world;
  SDL_Renderer *renderer;

public:
  Renderer(SDL_Window *window, b2World *world) :
    window(window),
    world(world)
  {
    this->renderer = SDL_CreateRenderer(this->window, -1,
                                        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  }

  virtual ~Renderer() {
    SDL_DestroyRenderer(this->renderer);
  }

  void Render() {
    SDL_SetRenderDrawColor(this->renderer, 0, 0, 255, 255);
    SDL_RenderClear(this->renderer);

    for (b2Body *b = this->world->GetBodyList(); b; b = b->GetNext()) {
      for (b2Fixture *f = b->GetFixtureList(); f; f = f->GetNext()) {
        this->DrawDisk(b->GetPosition(), f->GetShape()->m_radius);
      }
    }

    SDL_RenderPresent(this->renderer);
  }

  void DrawDisk(b2Vec2 pos, float32 radius) {
    radius *= ppm;
    int x = (pos.x - camerax) * ppm;
    int y = (pos.y - cameray) * ppm;

    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    y = h - y;

    aacircleRGBA(this->renderer, x, y, radius, 255, 255, 255, 255);
    filledCircleRGBA(this->renderer, x, y, radius, 255, 255, 255, 255);
  }
};

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
                            SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
  if (window == nullptr) {
    cout << "Window could not be created. SDL_Error: "
         << SDL_GetError() << endl;
    return 2;
  }

  b2World world(b2Vec2(0.0, 0.0));

  // Initialize world.
  b2BodyDef bd;
  bd.type = b2_dynamicBody;
  bd.position.Set(0.0, 0.0);
  b2Body *body = world.CreateBody(&bd);

  b2CircleShape shape;
  shape.m_p.Set(0.0, 0.0);
  shape.m_radius = 10.0;

  b2FixtureDef fd;
  fd.shape = &shape;
  fd.friction = 1.0;
  fd.restitution = 0.0;
  fd.density = 1e10;
  b2Fixture *fixture = body->CreateFixture(&fd);

  Entity *e = new Entity { body, true, 1000.0 };
  body->SetUserData(e);

  bd.type = b2_dynamicBody;
  bd.position.Set(20.0, 20.0);
  body = world.CreateBody(&bd);

  shape.m_p.Set(0.0, 0.0);
  shape.m_radius = 2.0;

  fd.shape = &shape;
  fd.friction = 1.0;
  fd.restitution = 0.0;
  fd.density = 1.0;
  fixture = body->CreateFixture(&fd);

  e = new Entity { body, false, 0.0 };
  body->SetUserData(e);

  Renderer renderer(window, &world);

  bool paused = false;
  bool stepOnce = false;
  int x, y;

  b2Body *draggingBody = nullptr;
  b2Vec2 draggingOffset;

  uint32_t lastTime = SDL_GetTicks();

  while (!quit) {
    renderer.Render();

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) {
        quit = true;
      }
      else if (e.type == SDL_MOUSEBUTTONDOWN) {
        if (e.button.button == SDL_BUTTON_LEFT) {
          SDL_GetMouseState(&x, &y);
          b2Vec2 p = WindowToWorldCoords(x, y, window);
          b2Body *b = GetBodyFromPoint(p, &world);
          if (b) {
            Entity *e = (Entity*) b->GetUserData();
            if (e->isGravitySource) {
              draggingBody = b;
              draggingOffset = p - b->GetPosition();
            }
          }
        }
      }
      else if (e.type == SDL_MOUSEMOTION) {
        if (draggingBody) {
          SDL_GetMouseState(&x, &y);
          b2Vec2 p = WindowToWorldCoords(x, y, window);
          draggingBody->SetTransform(p - draggingOffset, 0.0);
        }
      }
      else if (e.type == SDL_MOUSEBUTTONUP) {
        if (e.button.button == SDL_BUTTON_LEFT) {
          draggingBody = nullptr;
        }
      }
      else if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
        case SDLK_q:
          quit = true;
          break;
        }
      }
    }

    vector<Entity*> gravitySources;

    for (b2Body *b = world.GetBodyList(); b; b = b->GetNext()) {
      Entity *e = (Entity*) b->GetUserData();
      if (e->isGravitySource) {
        gravitySources.push_back(e);
      }
    }



    for (b2Body *b = world.GetBodyList(); b; b = b->GetNext()) {
      b2Vec2 gravity;
      Entity *e = (Entity*) b->GetUserData();
      if (!e->isGravitySource) {
        for (auto e : gravitySources) {
          b2Vec2 n = e->body->GetPosition() - b->GetPosition();
          n.Normalize();
          gravity += e->gravityCoeff * n;
        }

        b->ApplyForce(gravity, b->GetWorldCenter(), true);
      }
    }

    float32 dt = (SDL_GetTicks() - lastTime);
    SDL_Delay(b2Abs(TIME_STEP - dt));
    dt = (SDL_GetTicks() - lastTime);
    if (!paused || (paused && stepOnce)) {
      world.Step(dt / 1000.0, 10, 10);
      stepOnce = false;
    }
    renderer.Render();
    lastTime = SDL_GetTicks();
  }

  // Destroy the window.
  SDL_DestroyWindow(window);

  //Quit SDL subsystems
  SDL_Quit();

  return 0;
}
