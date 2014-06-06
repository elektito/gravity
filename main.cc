#include "entity.hh"
#include "sdl-renderer.hh"
#include "camera.hh"
#include "timer.hh"
#include "game.hh"

#include <SDL2/SDL.h>
#include <Box2D/Box2D.h>

#include <iostream>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

const int TIME_STEP = 15;

const float32 min_width = 150;
const float32 min_height = 75;
const float32 max_width = 300;
const float32 max_height = 150;

using namespace std;

void UpdateTrail(b2Body *b, Trail *t, float32 currentTime) {
  // Remove all the points not in the desired time window.
  t->points.erase(remove_if(t->points.begin(), t->points.end(),
                            [=](const TrailPoint &p) -> bool {
                              return p.time < currentTime - t->time;
                            }),
                  t->points.end());

  // Add current position to the trail.
  t->points.push_back(TrailPoint { b->GetPosition(), currentTime });
}

void FixCamera(Camera *camera, SDL_Window *window, b2Body *body) {
  int winw, winh;
  SDL_GetWindowSize(window, &winw, &winh);
  float32 ratio = ((float32) winw) / winh;

  float32 width, height;

  b2Vec2 upper(camera->pos.x + winw / camera->ppm,
               camera->pos.y + winh / camera->ppm);
  b2Vec2 lower(camera->pos.x, camera->pos.y);

  auto r = body->GetFixtureList()->GetShape()->m_radius;

  auto pos = body->GetPosition();

  float32 maxx, maxy, minx, miny;

  if (pos.x + r + 2 > upper.x ||
      pos.y + r + 2 > upper.y ||
      pos.x - r - 2 < lower.x ||
      pos.y - r - 2 < lower.y)
  {
    maxx = max(pos.x + r + 2, upper.x);
    maxy = max(pos.y + r + 2, upper.y);
    minx = min(pos.x - r - 2, lower.x);
    miny = min(pos.y - r - 2, lower.y);
  }
  else {
    maxx = min(pos.x + r + 2, upper.x);
    maxy = min(pos.y + r + 2, upper.y);
    minx = max(pos.x - r - 2, lower.x);
    miny = max(pos.y - r - 2, lower.y);
  }

  auto halfx = max(abs(maxx), abs(minx));
  auto halfy = max(abs(maxy), abs(miny));

  auto width1 = 2 * halfx;
  auto height1 = width1 / ratio;

  auto height2 = 2 * halfy;
  auto width2 = height2 * ratio;

  if (width1 > width2) {
    width = width1;
    height = height1;
  }
  else {
    width = width2;
    height = height2;
  }

  if (width > max_width) {
    width = max_width;
    height = width / ratio;
  }
  if (width < min_width) {
    width = min_width;
    height = width / ratio;
  }
  if (height > max_height) {
    height = max_height;
    width = height * ratio;
  }
  if (height < min_height) {
    height = min_height;
    width = height * ratio;
  }

  camera->pos.x = - (width / 2.0);
  camera->pos.y = - (height / 2.0);

  camera->ppm = winw / width;
}

class ContactListener : public b2ContactListener {
public:
  virtual void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) {
    //float32 f = contact->GetFriction();
    //cout << "hit " << f << endl;
  }
};

b2Body *GetBodyFromPoint(b2Vec2 p, b2World *world) {
  for (b2Body *b = world->GetBodyList(); b; b = b->GetNext()) {
    for (b2Fixture *f = b->GetFixtureList(); f; f = f->GetNext()) {
      if (f->TestPoint(p))
        return b;
    }
  }

  return nullptr;
}

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

  b2World world(b2Vec2(0.0, 0.0));

  // Initialize world.
  b2BodyDef bd;
  bd.type = b2_dynamicBody;
  bd.position.Set(0.0, 0.0);
  b2Body *body = world.CreateBody(&bd);

  b2CircleShape shape;
  shape.m_p.Set(0.0, 0.0);
  shape.m_radius = 6.0;

  b2FixtureDef fd;
  fd.shape = &shape;
  fd.friction = 0.5;
  fd.restitution = 0.7;
  fd.density = 1000;
  b2Fixture *fixture = body->CreateFixture(&fd);

  Entity *e = new Entity { body, true, 130000.0 };
  body->SetUserData(e);

  bd.type = b2_dynamicBody;
  bd.position.Set(20.0, 20.0);
  body = world.CreateBody(&bd);

  shape.m_p.Set(0.0, 0.0);
  shape.m_radius = 2.0;

  fd.shape = &shape;
  fd.friction = 0.5;
  fd.restitution = 0.7;
  fd.density = 1.0;
  fixture = body->CreateFixture(&fd);

  e = new Entity { body, false, 0.0 };
  body->SetUserData(e);

  ContactListener contactListener;
  world.SetContactListener(&contactListener);

  Trail trail;
  trail.size = 30;
  trail.time = 1.0;
  trail.body = world.GetBodyList();

  Camera camera;
  camera.pos.Set(-50.0, -50.0);
  camera.ppm = 10.0;

  Game game;
  game.score = 0;
  game.timeRemaining = 120;
  Timer t([&](float elapsed) { if (game.timeRemaining > 0) game.timeRemaining -= 1; });
  t.Set(1.0, true);
  Timer::PauseAll();

  Renderer *renderer = SdlRenderer::Create(window, &camera, &world, &trail, &game);

  bool paused = true;
  bool stepOnce = false;
  int x, y;

  b2Body *draggingBody = nullptr;
  b2Vec2 draggingOffset;

  uint32_t lastTime = SDL_GetTicks();

  float32 startTime = SDL_GetTicks() / 1000.0;
  while (!quit) {
    FixCamera(&camera, window, world.GetBodyList());

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) {
        quit = true;
      }
      else if (e.type == SDL_MOUSEBUTTONDOWN) {
        if (e.button.button == SDL_BUTTON_LEFT) {
          SDL_GetMouseState(&x, &y);
          b2Vec2 p = camera.PointToWorld(x, y, window);
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
          b2Vec2 p = camera.PointToWorld(x, y, window);
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
        case SDLK_p:
          paused = !paused;
          Timer::TogglePauseAll();
          break;
        case SDLK_n:
          stepOnce = true;
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

    Timer::CheckAll();

    if (!paused || (paused && stepOnce))
      UpdateTrail(world.GetBodyList(), &trail, SDL_GetTicks() / 1000.0 - startTime);

    if (!paused || (paused && stepOnce)) {
      vector<Entity*> gravitySources;

      for (b2Body *b = world.GetBodyList(); b; b = b->GetNext()) {
        Entity *e = (Entity*) b->GetUserData();
        if (e->isGravitySource) {
          gravitySources.push_back(e);
        }
      }

      // Update score.
      if (!paused || (paused && stepOnce)) {
        auto body = world.GetBodyList();
        auto sun = body->GetNext();
        auto v = body->GetLinearVelocityFromWorldPoint(body->GetPosition()).Length();
        auto d = (body->GetPosition() - sun->GetPosition()).Length();
        if (d > 100) d = 0;
        int diff = v * d / 100;
        game.score += diff;
      }

      for (b2Body *b = world.GetBodyList(); b; b = b->GetNext()) {
        b2Vec2 gravity;
        Entity *e = (Entity*) b->GetUserData();
        if (!e->isGravitySource) {
          gravity.Set(0.0, 0.0);
          for (auto e : gravitySources) {
            b2Vec2 n = e->body->GetPosition() - b->GetPosition();
            float32 r2 = n.LengthSquared();
            n.Normalize();
            gravity += e->gravityCoeff / r2 * n;
          }

          b->ApplyForce(gravity, b->GetWorldCenter(), true);
        }
      }
    }

    float32 dt = (SDL_GetTicks() - lastTime);
    SDL_Delay(b2Abs(TIME_STEP - dt));
    dt = (SDL_GetTicks() - lastTime);
    if (!paused || (paused && stepOnce)) {
      world.Step(dt / 1000.0, 10, 10);
      stepOnce = false;
    }
    renderer->Render();
    lastTime = SDL_GetTicks();
  }

  delete renderer;

  // Destroy the window.
  SDL_DestroyWindow(window);

  // Quit SDL.
  SDL_Quit();

  return 0;
}
