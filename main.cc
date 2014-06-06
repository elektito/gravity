#include <iostream>
#include <sstream>
#include <iomanip>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_render.h>
#include <Box2D/Box2D.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

const int TIME_STEP = 15;

float32 ppm = 5.0;
float32 camerax = -50.0;
float32 cameray = -50.0;

const float32 min_width = 150;
const float32 min_height = 75;
const float32 max_width = 300;
const float32 max_height = 150;

using namespace std;

struct TrailPoint {
  b2Vec2 pos;
  float32 time;
};

struct Trail {
  b2Body *body;
  int size;
  float32 time;
  vector<TrailPoint> points;
} trail;

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

#define max(x, y) ((x) > (y) ? (x) : (y))
#define min(x, y) ((x) < (y) ? (x) : (y))
#define abs(x) ((x) > 0 ? (x) : -(x))

void FixCamera(SDL_Window *window, b2Body *body) {
  int winw, winh;
  SDL_GetWindowSize(window, &winw, &winh);
  float32 ratio = ((float32) winw) / winh;

  float32 width, height;

  b2Vec2 upper(camerax + winw / ppm, cameray + winh / ppm);
  b2Vec2 lower(camerax, cameray);

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

  camerax = - (width / 2.0);
  cameray = - (height / 2.0);

  ppm = winw / width;
}

void PointToScreen(SDL_Window *window, b2Vec2 p, int &x, int &y) {
  x = (p.x - camerax) * ppm;
  y = (p.y - cameray) * ppm;

  int w, h;
  SDL_GetWindowSize(window, &w, &h);
  y = h - y;
}

float32 LengthToScreen(float32 length) {
  return length * ppm;
}

class ContactListener : public b2ContactListener {
public:
  virtual void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) {
    //float32 f = contact->GetFriction();
    //cout << "hit " << f << endl;
  }
};

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
  TTF_Font *font;

public:
  uint32 score;

  Renderer(SDL_Window *window, b2World *world) :
    window(window),
    world(world),
    score(0)
  {
    this->renderer = SDL_CreateRenderer(this->window, -1,
                                        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    // Initialize fonts
    if (TTF_Init() == -1) {
      cout << "Could not initialize SDL_ttf. SDL_ttf error: "
           << TTF_GetError() << endl;
      exit(3);
    }

    this->font = TTF_OpenFont("fonts/UbuntuMono-B.ttf", 72);
    if (font == nullptr) {
      cout << "Unable to load font. SDL_ttf error: "
           << TTF_GetError() << endl;
      exit(4);
    }
  }

  virtual ~Renderer() {
    TTF_CloseFont(this->font);
    SDL_DestroyRenderer(this->renderer);
  }

  void Render() {
    SDL_SetRenderDrawColor(this->renderer, 0, 0, 255, 255);
    SDL_RenderClear(this->renderer);

    this->DrawTrail(&trail);

    // Draw grid.
    int winw, winh;
    SDL_GetWindowSize(window, &winw, &winh);

    float32 upperx = camerax + winw / ppm;
    float32 uppery = cameray + winh / ppm;
    float32 x = camerax + fmod(camerax + 10, 10);
    float32 y = camerax + fmod(cameray + 10, 10);
    for (; x <= upperx; x += 10)
      this->DrawLine(b2Vec2(x, cameray), b2Vec2(x, uppery), 32, 64, 64, 255);
    for (; y <= uppery; y += 10)
      this->DrawLine(b2Vec2(camerax, y), b2Vec2(upperx, y), 32, 64, 64, 255);

    for (b2Body *b = this->world->GetBodyList(); b; b = b->GetNext()) {
      for (b2Fixture *f = b->GetFixtureList(); f; f = f->GetNext()) {
        this->DrawDisk(b->GetPosition(), f->GetShape()->m_radius, 255, 255, 255, 255);
      }
    }

    this->DrawLine(b2Vec2(0, 1), b2Vec2(0, -1), 255, 0, 0, 255);
    this->DrawLine(b2Vec2(1, 0), b2Vec2(-1, 0), 255, 0, 0, 255);

    this->DrawScore();

    SDL_RenderPresent(this->renderer);
  }

  void DrawDisk(b2Vec2 pos, float32 radius, int r, int g, int b, int a) {
    int x, y;
    radius = LengthToScreen(radius);
    PointToScreen(this->window, pos, x, y);

    aacircleRGBA(this->renderer, x, y, radius, r, g, b, a);
    filledCircleRGBA(this->renderer, x, y, radius, r, g, b, a);
  }

  void DrawLine(b2Vec2 begin, b2Vec2 end, int r, int g, int b, int a) {
    int x1, y1, x2, y2;
    PointToScreen(this->window, begin, x1, y1);
    PointToScreen(this->window, end, x2, y2);

    SDL_SetRenderDrawColor(this->renderer, r, g, b, a);
    SDL_RenderDrawLine(this->renderer, x1, y1, x2, y2);
  }

  void DrawText(string text, SDL_Color color) {
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, text.data(), color);
    if (textSurface == nullptr) {
      cout << "Unable to render text surface. SDL_ttf error: "
           << TTF_GetError() << endl;
      return;
    }

    auto texture = SDL_CreateTextureFromSurface(this->renderer, textSurface);
    if (texture == nullptr) {
      cout << "Unable to create texture from rendered text. SDL error: "
           << SDL_GetError() << endl;
    }

    SDL_FreeSurface(textSurface);

    if (texture) {
      int winw, winh;
      SDL_GetWindowSize(this->window, &winw, &winh);
      SDL_Rect dest = {winw - textSurface->w - 10, 10, textSurface->w, textSurface->h};
      SDL_RenderCopy(this->renderer, texture, nullptr, &dest);
      SDL_DestroyTexture(texture);
    }
  }

  void DrawScore() {
    stringstream ss;
    ss << setw(6) << setfill('0') << this->score;
    this->DrawText(ss.str(), SDL_Color {0, 0, 0, 128});
  }

  void DrawTrail(Trail *t) {
    vector<TrailPoint> points;

    if (t->size < t->points.size()) {
      // Choose 't->size' points in the 't->time' time-window.

      // From the rest choose enough, as evenly timed as possible.
      auto step = t->time / t->size;
      auto time = t->points[0].time;
      auto it = t->points.begin();
      while (points.size() < t->size) {
        time += step;

        // Go forward among previous locations until we reach one after
        // 'time'. Choose the point as close to the time we want as
        // possible.
        float32 leastDiff = FLT_MAX;
        TrailPoint closestPoint;
        for (; it != t->points.end(); ++it) {
          if (abs(it->time - time) < leastDiff) {
            leastDiff = abs(it->time - time);
            closestPoint = *it;
          }

          if (it->time >= time)
            break;
        }

        points.push_back(closestPoint);

        // Continue from this point.
        time = closestPoint.time;
      }
    }
    else
      points = t->points;

    float32 r = t->body->GetFixtureList()->GetShape()->m_radius;
    auto startr = r / 10.0;
    auto endr = r / 2.0;
    r = startr;
    float32 a = 128;
    float32 dr, da;
    if (points.size() > 0) {
      dr = (endr - startr) / points.size();
      da = (255 - a) / points.size();
    }

    for (auto &p : points) {
      this->DrawDisk(p.pos, r, 255, 0, 0, a);
      r += dr;
      a += da;
    }
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

  Renderer *renderer = new Renderer(window, &world);

  bool paused = true;
  bool stepOnce = false;
  int x, y;

  b2Body *draggingBody = nullptr;
  b2Vec2 draggingOffset;

  uint32_t lastTime = SDL_GetTicks();

  float32 startTime = SDL_GetTicks() / 1000.0;
  trail.size = 30;
  trail.time = 1.0;
  trail.body = world.GetBodyList();
  while (!quit) {
    FixCamera(window, world.GetBodyList());

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
        case SDLK_p:
          paused = !paused;
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
        renderer->score += diff;
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

  //Quit SDL subsystems
  TTF_Quit();
  SDL_Quit();

  return 0;
}
