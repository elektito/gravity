#include "game-screen.hh"
#include "entity.hh"

#include <sstream>
#include <iomanip>
#include <functional>

using std::placeholders::_1;

using namespace std;

ContactListener::ContactListener(GameScreen *screen) :
  screen(screen),
  inContact(false)
{}

void ContactListener::BeginContact(b2Contact *contact) {
  if (!this->inContact)
    this->screen->timeRemaining -= 10;
  this->inContact = true;
}

void ContactListener::EndContact(b2Contact *contact) {
  this->inContact = false;
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

GameScreen::GameScreen(SDL_Window *window) :
  Screen(window),
  world(b2Vec2(0.0, 0.0)),
  timer(bind(&GameScreen::TimerCallback, this, _1)),
  contactListener(this),
  frameCount(0),
  fps(0)
{
  this->timer.Set(1.0, true);

  this->world.SetContactListener(&contactListener);

  this->Reset();
}

GameScreen::~GameScreen() {

}

void GameScreen::HandleEvent(const SDL_Event &e) {
  int x, y;

  if (e.type == SDL_MOUSEBUTTONDOWN) {
    if (e.button.button == SDL_BUTTON_LEFT) {
      SDL_GetMouseState(&x, &y);
      b2Vec2 p = this->camera.PointToWorld(x, y, this->window);
      b2Body *b = GetBodyFromPoint(p, &this->world);
      if (b) {
        Entity *e = (Entity*) b->GetUserData();
        if (e->isSun) {
          this->draggingBody = b;
          this->draggingOffset = p - b->GetPosition();
        }
      }
    }
  }
  else if (e.type == SDL_MOUSEMOTION) {
    if (this->draggingBody) {
      SDL_GetMouseState(&x, &y);
      b2Vec2 p = this->camera.PointToWorld(x, y, this->window);
      this->draggingBody->SetTransform(p - this->draggingOffset, 0.0);
    }
  }
  else if (e.type == SDL_MOUSEBUTTONUP) {
    if (e.button.button == SDL_BUTTON_LEFT) {
      this->draggingBody = nullptr;
    }
  }
  else if (e.type == SDL_KEYDOWN) {
    switch (e.key.keysym.sym) {
    case SDLK_q:
      SDL_Event quitEvent;
      quitEvent.type = SDL_QUIT;
      SDL_PushEvent(&quitEvent);
      break;
    case SDLK_p:
      this->paused = !this->paused;
      Timer::TogglePauseAll();
      break;
    case SDLK_n:
      this->stepOnce = true;
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
  else if (e.type == SDL_WINDOWEVENT) {
    if (e.window.event == SDL_WINDOWEVENT_RESIZED)
      this->FixCamera();
  }
}

void GameScreen::Reset() {
  this->score = 0;
  this->timeRemaining = 120;
  this->paused = true;
  this->time = 0.0;

  this->camera.pos.Set(-50.0, -50.0);
  this->camera.ppm = 10.0;

  this->draggingBody = nullptr;
  this->stepOnce = false;

  this->sun = Entity::CreateSun(&this->world,
                                b2Vec2(0.0, 0.0),
                                6.0,
                                1000.0,
                                130000.0);
  this->entities.push_back(this->sun);

  this->entities.push_back(Entity::CreatePlanet(&this->world,
                                                b2Vec2(20.0, 20.0),
                                                2.0,
                                                1.0));

  Timer::PauseAll();
  this->FixCamera();
}

void GameScreen::Save(ostream &s) const {
  s << this->time
    << this->score
    << this->timeRemaining
    << this->paused
    << this->camera.pos.x
    << this->camera.pos.y
    << this->camera.ppm;

  s << this->entities.size();
  for (auto e : this->entities)
    e->Save(s);
}

void GameScreen::Load(istream &s) {
  s >> this->time
    >> this->score
    >> this->timeRemaining
    >> this->paused
    >> this->camera.pos.x
    >> this->camera.pos.y
    >> this->camera.ppm;

  this->entities.clear();
  this->world = b2World(b2Vec2(0.0, 0.0));
  Entity *e;
  size_t entityCount;
  s >> entityCount;
  for (int i = 0; i < entityCount; ++i) {
    e = new Entity;
    e->Load(s, &this->world);
    this->entities.push_back(e);

    if (e->isSun)
      this->sun = e;
  }

  if (this->paused)
    Timer::PauseAll();
  else
    Timer::UnpauseAll();
}

void GameScreen::Advance(float dt) {
  if (this->paused && !this->stepOnce)
    return;

  this->time += dt;

  Timer::CheckAll();

  this->FixCamera();

  // Update the trails.
  UpdateTrails();

  // Update score.
  for (auto e : this->entities)
    if (e->isPlanet) {
      auto v = e->body->GetLinearVelocityFromWorldPoint(e->body->GetPosition()).Length();
      auto d = (e->body->GetPosition() - this->sun->body->GetPosition()).Length();
      if (d > 100) d = 0;
      int diff = v * d / 100;
      this->score += diff;
    }

  // Apply forces.
  vector<Entity*> gravitySources;

  for (auto e : this->entities)
    if (e->hasGravity)
      gravitySources.push_back(e);

  b2Vec2 gravity;
  for (auto e : this->entities) {
    if (e->isAffectedByGravity) {
      gravity.Set(0.0, 0.0);
      for (auto s : gravitySources) {
        b2Vec2 n = s->body->GetPosition() - e->body->GetPosition();
        float32 r2 = n.LengthSquared();
        n.Normalize();
        gravity += s->gravityCoeff / r2 * n;
      }

      e->body->ApplyForce(gravity, e->body->GetWorldCenter(), true);
    }
  }

  // Advance physics.
  this->world.Step(dt, 10, 10);
  this->stepOnce = false;
}

void GameScreen::Render(Renderer *renderer) {
  renderer->SetCamera(this->camera);

  renderer->DrawBackground();
  renderer->DrawGrid();

  for (auto e : this->entities)
    if (e->hasTrail)
      renderer->DrawTrail(e);

  for (const b2Body *b = this->world.GetBodyList(); b; b = b->GetNext()) {
    renderer->DrawEntity((Entity*) b->GetUserData());
  }

  // Draw HUD.

  // Draw origin.
  renderer->DrawLine(b2Vec2(0, 1), b2Vec2(0, -1), 255, 0, 0, 255);
  renderer->DrawLine(b2Vec2(1, 0), b2Vec2(-1, 0), 255, 0, 0, 255);

  // Draw score.
  stringstream ss;
  ss << setw(6) << setfill('0') << this->score;
  renderer->DrawText(ss.str(), SDL_Color {0, 0, 0, 128}, 10, 10, false, true);

  // Draw time.
  int minutes = this->timeRemaining / 60;
  int seconds = this->timeRemaining % 60;
  stringstream ss2;
  ss2 << setw(2) << setfill('0') << minutes
      << setw(0) << ":"
      << setw(2) << setfill('0') << seconds;
  renderer->DrawText(ss2.str(), SDL_Color {0, 0, 0, 128}, 10, 10);

  // Draw FPS Counter.
  if (!this->paused) {
    stringstream ss3;
    ss3 << "FPS: " << this->fps;
    renderer->DrawText(ss3.str(), SDL_Color {0, 0, 0, 128}, 10, 10, true, false);

    this->frameCount++;
  }

  renderer->PresentScreen();
}

void GameScreen::FixCamera() {
  for (auto e : this->entities)
    this->FixCamera(e);
}

void GameScreen::FixCamera(Entity *e) {
  const float32 min_width = 150;
  const float32 min_height = 75;
  const float32 max_width = 300;
  const float32 max_height = 150;

  int winw, winh;
  SDL_GetWindowSize(this->window, &winw, &winh);
  float32 ratio = ((float32) winw) / winh;

  float32 width, height;

  b2Vec2 upper(this->camera.pos.x + winw / this->camera.ppm,
               this->camera.pos.y + winh / this->camera.ppm);
  b2Vec2 lower(this->camera.pos.x, this->camera.pos.y);

  auto r = e->body->GetFixtureList()->GetShape()->m_radius;

  auto pos = e->body->GetPosition();

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

  this->camera.pos.x = - (width / 2.0);
  this->camera.pos.y = - (height / 2.0);

  this->camera.ppm = winw / width;
}

void GameScreen::UpdateTrails() {
  for (auto e : this->entities)
    if (e->hasTrail) {
      // Remove all the points not in the desired time window.
      e->trail.points.erase(remove_if(e->trail.points.begin(), e->trail.points.end(),
                                      [=](const TrailPoint &p) -> bool {
                                        return p.time < this->time - e->trail.time;
                                      }),
                            e->trail.points.end());

      // Add current position to the trail.
      e->trail.points.push_back(TrailPoint { e->body->GetPosition(), this->time });
    }
}

void GameScreen::TimerCallback(float elapsed) {
  // Decrement remaining time.
  if (this->timeRemaining > 0)
    this->timeRemaining--;

  // Update FPS counter.
  this->fps = this->frameCount;
  this->frameCount = 0;
}
