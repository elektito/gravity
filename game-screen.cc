#include "game-screen.hh"
#include "entity.hh"
#include "helpers.hh"
#include "resource-cache.hh"
#include "config.hh"

#include <sstream>
#include <iomanip>
#include <functional>
#include <algorithm>

#define M_PI 3.14159265358979323846

using std::placeholders::_1;

using namespace std;

b2Vec2 RotatePoint(b2Vec2 point, float theta, b2Vec2 center) {
  b2Vec2 np;
  point = point - center;
  np.x = point.x * cos(theta) - point.y * sin(theta);
  np.y = point.x * sin(theta) + point.y * cos(theta);
  return np + center;
}

ContactListener::ContactListener(GameScreen *screen) :
  screen(screen),
  inContact(false)
{}

void ContactListener::BeginContact(b2Contact *contact) {
  Entity *e1 = (Entity*) contact->GetFixtureA()->GetBody()->GetUserData().pointer;
  Entity *e2 = (Entity*) contact->GetFixtureB()->GetBody()->GetUserData().pointer;

  if (e1->isSun && e2->isPlanet)
    this->PlanetSunContact(e2, e1);
  if (e2->isSun && e1->isPlanet)
    this->PlanetSunContact(e1, e2);

  if (e1->isCollectible && e2->isSun)
    this->CollectibleSunContact(e1, e2);
  if (e2->isCollectible && e1->isSun)
    this->CollectibleSunContact(e2, e1);

  if (e1->isCollectible && e2->isPlanet)
    this->CollectiblePlanetContact(e1, e2);
  if (e2->isCollectible && e1->isPlanet)
    this->CollectiblePlanetContact(e2, e1);

  if (e1->isEnemy && e2->isSun)
    this->EnemySunContact(e1, e2);
  if (e2->isEnemy && e1->isSun)
    this->EnemySunContact(e2, e1);

  if (e1->isEnemy && e2->isPlanet)
    this->EnemyPlanetContact(e1, e2);
  if (e2->isEnemy && e1->isPlanet)
    this->EnemyPlanetContact(e2, e1);
}

void ContactListener::EnemySunContact(Entity *enemy, Entity *sun) {
  PlaySound("enemy-collision");

  this->screen->SetTimeRemaining(this->screen->timeRemaining - 10);
  if (this->screen->timeRemaining < 0)
    this->screen->SetTimeRemaining(0);

  this->screen->toBeRemoved.push_back(enemy);
}

void ContactListener::EnemyPlanetContact(Entity *enemy, Entity *sun) {
  PlaySound("enemy-collision");

  this->screen->SetTimeRemaining(this->screen->timeRemaining - 10);
  if (this->screen->timeRemaining < 0)
    this->screen->SetTimeRemaining(0);

  this->screen->toBeRemoved.push_back(enemy);
}

void ContactListener::PlanetSunContact(Entity *planet, Entity *sun) {
  PlaySound("planet-sun-collision");

  if (!this->inContact)
    this->screen->SetTimeRemaining(this->screen->timeRemaining - 10);
  if (this->screen->timeRemaining < 0)
    this->screen->SetTimeRemaining(0);

  this->inContact = true;
}

void ContactListener::CollectibleSunContact(Entity *collectible, Entity *sun) {
  PlaySound("sun-powerup");

  if (collectible->hasScore)
    this->screen->SetScore(this->screen->score + collectible->score);

  if (collectible->hasTime)
    this->screen->SetTimeRemaining(this->screen->timeRemaining + collectible->time);

  if (collectible->spawnPlanet)
    this->screen->spawnPlanet = true;

  this->screen->toBeRemoved.push_back(collectible);
}

void ContactListener::CollectiblePlanetContact(Entity *collectible, Entity *planet) {
  PlaySound("planet-powerup");

  if (collectible->hasScore)
    this->screen->SetScore(this->screen->score + 10 * collectible->score);

  if (collectible->hasTime)
    this->screen->SetTimeRemaining(this->screen->timeRemaining + 2 * collectible->time);

  if (collectible->spawnPlanet)
    this->screen->spawnPlanet = true;

  this->screen->toBeRemoved.push_back(collectible);
}

void ContactListener::EndContact(b2Contact *contact) {
  this->inContact = false;
}

bool ContactFilter::ShouldCollide(b2Fixture *fixtureA, b2Fixture *fixtureB) {
  Entity *e1 = (Entity*) fixtureA->GetBody()->GetUserData().pointer;
  Entity *e2 = (Entity*) fixtureB->GetBody()->GetUserData().pointer;

  if (e1->isEnemy && e2->isEnemy)
    return false;
  if (e1->isEnemy && e2->isCollectible)
    return false;
  if (e2->isEnemy && e1->isCollectible)
    return false;
  if (e2->isCollectible && e1->isCollectible)
    return false;

  return true;
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
  fps(0),
  spawnPlanet(false),
  background(window, ResourceCache::GetTexture("background")),
  discardLeftButtonUp(false)
{
  this->timer.Set(1.0, true);

  this->world.SetContactListener(&this->contactListener);
  this->world.SetContactFilter(&this->contactFilter);

  this->scoreLabel = new NumberWidget(this,
                                      0,
                                      0.02, 0.025, 0.0655,
                                      TextAnchor::RIGHT, TextAnchor::TOP,
                                      6,
                                      {255, 255, 255, 128});

  this->timeLabel = new LabelWidget(this,
                                     "02:00",
                                     0.02, 0.0, 0.1,
                                     TextAnchor::LEFT, TextAnchor::TOP,
                                     {255, 255, 255, 128});
#ifndef RELEASE_BUILD
  this->fpsLabel = new LabelWidget(this,
                                   "FPS: 0",
                                   0.02, 0.0, 0.1,
                                   TextAnchor::LEFT, TextAnchor::BOTTOM,
                                   {255, 255, 255, 128});
#endif
  this->continueLabel = new ImageWidget(this,
                                        ResourceCache::GetTexture("continue"),
                                        0.0, -0.15, 0.05,
                                        TextAnchor::CENTER, TextAnchor::CENTER,
                                        {255, 255, 255, 128});
  this->pauseSign = new ImageWidget(this,
                                    ResourceCache::GetTexture("pause"),
                                    0.0, 0.0, 0.2,
                                    TextAnchor::CENTER, TextAnchor::CENTER,
                                    {255, 255, 255, 128});
  this->endGameButton = new ImageButtonWidget(this,
                                              ResourceCache::GetTexture("end-game"),
                                              0.02, 0.02, 0.05,
                                              TextAnchor::RIGHT, TextAnchor::BOTTOM,
                                              {255, 0, 0, 128},
                                              {255, 255, 255, 128});
  this->muteButton = new ImageButtonWidget(this,
                                           ResourceCache::GetTexture("mute"),
                                           0.05, 0.05, 0.08,
                                           TextAnchor::LEFT, TextAnchor::BOTTOM,
                                           {255, 128, 128, 255},
                                           {255, 255, 255, 128});
  this->gameOverLabel = new ImageWidget(this,
                                        ResourceCache::GetTexture("game-over"),
                                        0.0, 0.0, 0.1,
                                        TextAnchor::CENTER, TextAnchor::CENTER);

  this->livesLabel = new ImageWidget(this,
                                     ResourceCache::GetTexture("lives3"),
                                     0.0, 0.0, 0.04,
                                     TextAnchor::CENTER, TextAnchor::TOP,
                                     {255, 255, 255, 255});

  this->widgets.push_back(this->scoreLabel);
  this->widgets.push_back(this->timeLabel);
#ifndef RELEASE_BUILD
  this->widgets.push_back(this->fpsLabel);
#endif
  this->widgets.push_back(this->continueLabel);
  this->widgets.push_back(this->pauseSign);
  this->widgets.push_back(this->endGameButton);
  this->widgets.push_back(this->muteButton);
  this->widgets.push_back(this->gameOverLabel);
  this->widgets.push_back(this->livesLabel);

  // Create the "trail point" mesh.
  GLfloat trailPointVertexData[] = {
    // triangle 1
    /* coord */ -2.0f, -2.0f, /* tex_coord */ 0.0f, 0.0f,
    /* coord */ -2.0f,  2.0f, /* tex_coord */ 0.0f, 1.0f,
    /* coord */  2.0f, -2.0f, /* tex_coord */ 1.0f, 0.0f,

    // triangle 2
    /* coord */ -2.0f,  2.0f, /* tex_coord */ 0.0f, 1.0f,
    /* coord */  2.0f,  2.0f, /* tex_coord */ 1.0f, 1.0f,
    /* coord */  2.0f, -2.0f, /* tex_coord */ 1.0f, 0.0f,
  };

  this->trailPointMesh = new Mesh(trailPointVertexData, 6, ResourceCache::GetTexture("trail-point"));

  // Reset all state data.
  this->Reset();
}

GameScreen::~GameScreen() {
  // Remove existing entities.
  for (auto e : this->entities) {
    if (e->hasPhysics)
      this->world.DestroyBody(e->body);
    delete e;
  }
  this->entities.clear();

  delete this->trailPointMesh;
}

void GameScreen::DiscardPlanet(Entity *planet) {
  int nplanets = 0;
  for (auto e : this->entities)
    if (e->isPlanet)
      nplanets++;

  if (nplanets == 1)
    this->spawnPlanet = true;

  this->toBeRemoved.push_back(planet);
  this->DecreaseLives();
}

void GameScreen::DecreaseLives() {
  int nplanets = 0;
  for (auto e : this->entities)
    if (e->isPlanet)
      nplanets++;

  if (nplanets > 1)
    return;

  if (this->lives == 0) {
    this->gameOverLabel->SetVisible(true);
    return;
  }

  this->lives--;
  switch (this->lives) {
  case 0:
    this->livesLabel->SetTexture(ResourceCache::GetTexture("lives0"));
    break;
  case 1:
    this->livesLabel->SetTexture(ResourceCache::GetTexture("lives1"));
    break;
  case 2:
    this->livesLabel->SetTexture(ResourceCache::GetTexture("lives2"));
    break;
  case 3:
    this->livesLabel->SetTexture(ResourceCache::GetTexture("lives3"));
    break;
  }
}

void GameScreen::TogglePause() {
  this->paused = !this->paused;
#ifndef RELEASE_BUILD
  this->fpsLabel->SetVisible(!this->paused);
#endif
  this->continueLabel->SetVisible(this->paused);
  this->pauseSign->SetVisible(this->paused);
  this->endGameButton->SetVisible(this->paused);
  this->muteButton->SetVisible(this->paused);

  for (auto e : this->entities)
    if (e->isPlanet)
      if (this->paused)
        Mix_Pause(e->planetWhooshChannel);
      else
        Mix_Resume(e->planetWhooshChannel);

  if (this->paused) {
    this->draggingBody = nullptr;
  }

  Timer::TogglePauseAll();
}

void GameScreen::SetScore(int score) {
  if (score < 0)
    score = 0;
  this->score = score;
  this->scoreLabel->SetNumber(score);
}

void GameScreen::SetTimeRemaining(int time) {
  this->timeRemaining = time;
  if (this->timeRemaining < 0)
    this->timeRemaining = 0;

  int minutes = this->timeRemaining / 60;
  int seconds = this->timeRemaining % 60;
  stringstream ss;
  ss << setw(2) << setfill('0') << minutes
     << setw(0) << ":"
     << setw(2) << setfill('0') << seconds;
  this->timeLabel->SetText(ss.str());

  // Check for game over.
  if (this->timeRemaining == 0) {
    this->gameOverLabel->SetVisible(true);

    for (auto e : this->entities)
      if (e->isPlanet)
        Mix_Pause(e->planetWhooshChannel);

    return;
  }
}

b2Vec2 GameScreen::GetRandomPosition() {
  const float MIN_DISTANCE = 8;

  int wpixels, hpixels;
  SDL_GetWindowSize(this->window, &wpixels, &hpixels);

  // Get window dimensions in meters.
  float width = wpixels / this->camera.ppm;
  float height = hpixels / this->camera.ppm;

  b2Vec2 pos;
  while (true) {
    // Choose a random position for the collectible.
    pos.x = this->camera.pos.x + (float) rand() / RAND_MAX * width;
    pos.y = this->camera.pos.y + (float) rand() / RAND_MAX * height;

    // Retry if the chosen position is to close to a sun or a planet.
    for (auto e : this->entities)
      if (e->isSun || e->isPlanet) {
        // Get sun/planet radius.
        float r = e->body->GetFixtureList()->GetShape()->m_radius;

        // If distance from the surface (indicated by the '+r') is
        // less than minimum distance, retry.
        if ((e->body->GetPosition() - pos).Length() + r < MIN_DISTANCE);
          continue;
      }

    break;
  }

  return pos;
}

void GameScreen::SpawnPlanet() {
  b2Vec2 pos = this->GetRandomPosition();

  // Set the initial velocity such that the new planet seems to be
  // thrown to a point near the sun. (The following is perhaps not the
  // best method, but the first thing that came to my mind!)
  b2Vec2 v0 = this->sun->body->GetPosition() - pos; // a vector from
                                                    // the planet to
                                                    // the sun
  v0.Set(v0.y, -v0.x); // make a vector perpendicular to it.
  v0.Normalize(); // normalize it
  v0 *= 15.0; // and then make it length
  v0 += this->sun->body->GetPosition() - pos; // add it to the initial
                                              // vector. we now have a
                                              // vector towards a
                                              // point in the vicinity
                                              // of the sun.
  v0.Normalize(); // normalize it
  v0 *= 25; // and set the initial speed.

  Entity *planet = Entity::CreatePlanet(&this->world,
                                        pos,
                                        2.0,
                                        1.0,
                                        v0);
  this->entities.push_back(planet);
}

void GameScreen::SwitchScreen(const map<string, string> &lastState) {
  if (mute)
    this->muteButton->SetTexture(ResourceCache::GetTexture("unmute"));
  else
    this->muteButton->SetTexture(ResourceCache::GetTexture("mute"));
}

void GameScreen::HandleEvent(const SDL_Event &e) {
  int x, y;

  if (this->gameOverLabel->GetVisible())
    return;

  for (auto w : this->widgets)
    w->HandleEvent(e);

  switch (e.type) {
  case SDL_MOUSEBUTTONDOWN:
    if (e.button.button == SDL_BUTTON_LEFT) {
      SDL_GetMouseState(&x, &y);
      b2Vec2 p = this->camera.PointToWorld(x, y, this->window);
      b2Body *b = GetBodyFromPoint(p, &this->world);
      if (b) {
        Entity *e = (Entity*) b->GetUserData().pointer;
        if (e->isSun && !this->paused) {
          this->draggingBody = b;
          this->draggingOffset = p - b->GetPosition();
        }
      }

      this->mouseDown = true;
      this->mouseDownX = x;
      this->mouseDownY = y;
    }
    break;

  case SDL_MOUSEMOTION:
    if (this->draggingBody) {
      SDL_GetMouseState(&x, &y);
      b2Vec2 p = this->camera.PointToWorld(x, y, this->window);
      this->draggingBody->SetTransform(p - this->draggingOffset, 0.0);
    }
    break;

  case SDL_MOUSEBUTTONUP:
    if (e.button.button == SDL_BUTTON_LEFT) {
      this->draggingBody = nullptr;

      SDL_GetMouseState(&x, &y);
      if (mouseDown && abs(mouseDownX - x) <= 2 && abs(mouseDownY - y) <= 2) { // It's a click.
        if (!this->discardLeftButtonUp && this->paused)
          TogglePause();
        this->discardLeftButtonUp = false;
        this->mouseDown = false;
      }
    }
    break;

  case SDL_KEYDOWN:
    switch (e.key.keysym.sym) {
    case SDLK_p:
    case SDLK_ESCAPE:
      this->TogglePause();
      break;
    case SDLK_n:
      this->stepOnce = true;
      break;
    }
    break;

  case SDL_WINDOWEVENT:
    if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
      this->FixCamera();
    }
    break;
  } // switch (e.type)
}

void GameScreen::HandleWidgetEvent(int event_type, Widget *widget) {
  switch (event_type) {
  case BUTTON_CLICK:
    PlaySound("button-click");

    if (widget == this->endGameButton) { // End Game
      this->state["name"] = "game-over";
      this->state["score"] = to_string(this->score);
    }
    else if (widget == this->muteButton) { // Toggle Mute
      mute = !mute;
      if (mute)
        this->muteButton->SetTexture(ResourceCache::GetTexture("unmute"));
      else
        this->muteButton->SetTexture(ResourceCache::GetTexture("mute"));
    }
    this->discardLeftButtonUp = true;

    break;

  case BUTTON_MOUSE_ENTER:
    PlaySound("mouse-over");
    break;
  } // switch (event_type)
}

void GameScreen::Reset() {
  this->state.clear();
  this->state["name"] = "playing";

  this->SetScore(0);
  this->SetTimeRemaining(Config::GameTime);
  this->paused = true;
  this->time = 0.0;

  this->camera.pos.Set(-50.0, -50.0);
  this->camera.ppm = 10.0;

  this->draggingBody = nullptr;
  this->stepOnce = false;
  this->physicsTimeAccumulator = 0.0;
  this->scoreAccumulator = 0;
  this->lives = 3;
  this->livesLabel->SetTexture(ResourceCache::GetTexture("lives3"));
  this->spawnPlanet = false;

  // Remove existing entities.
  for (auto e : this->entities) {
    if (e->hasPhysics)
      this->world.DestroyBody(e->body);
    delete e;
  }
  this->entities.clear();
  this->toBeRemoved.clear();

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

  for (auto w : this->widgets)
    w->Reset();

#ifndef RELEASE_BUILD
  this->fpsLabel->SetVisible(!this->paused);
#endif
  this->continueLabel->SetVisible(this->paused);
  this->pauseSign->SetVisible(this->paused);
  this->endGameButton->SetVisible(this->paused);
  this->muteButton->SetVisible(this->paused);
  this->gameOverLabel->SetVisible(false);
}

void GameScreen::Save(ostream &s) const {
  SaveMap(this->state, s);

  WRITE(this->time, s);
  WRITE(this->score, s);
  WRITE(this->timeRemaining, s);
  WRITE(this->paused, s);
  WRITE(this->camera.pos.x, s);
  WRITE(this->camera.pos.y, s);
  WRITE(this->camera.ppm, s);
  WRITE(this->physicsTimeAccumulator, s);
  WRITE(this->scoreAccumulator, s);
  WRITE(this->lives, s);
  WRITE(this->spawnPlanet, s);

  size_t size = this->entities.size();
  WRITE(size, s);
  for (auto e : this->entities)
    e->Save(s);
}

void GameScreen::Load(istream &s) {
  LoadMap(this->state, s);

  READ(this->time, s);
  READ(this->score, s);
  this->SetScore(this->score);
  READ(this->timeRemaining, s);
  this->SetTimeRemaining(this->timeRemaining);
  READ(this->paused, s);
  READ(this->camera.pos.x, s);
  READ(this->camera.pos.y, s);
  READ(this->camera.ppm, s);
  READ(this->physicsTimeAccumulator, s);
  READ(this->scoreAccumulator, s);
  READ(this->lives, s);
  READ(this->spawnPlanet, s);

  this->entities.clear();
  this->world = b2World(b2Vec2(0.0, 0.0));
  Entity *e;
  size_t entityCount;
  READ(entityCount, s);
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
  Timer::CheckAll();

  if (this->gameOverLabel->GetVisible())
    return;

  for (auto w : this->widgets)
    w->Advance(dt);

  if (this->state["name"] == "game-over")
    return;

  if (this->paused && !this->stepOnce)
    return;

  // Set planet "whooshing" volume.
  for (auto e : this->entities)
    if (e->isPlanet) {
      float MIN_DISTANCE = 30.0f;
      float MIN_SPEED = 20.0f;
      float MAX_SPEED = 45.0f;

      int vol = 0;
      float speed = (e->body->GetLinearVelocity() - this->sun->body->GetLinearVelocity()).Length();
      if (speed < MIN_SPEED)
        vol = 0;
      else if (speed > MAX_SPEED)
        vol = MIX_MAX_VOLUME;
      else
        vol = MIX_MAX_VOLUME * (speed - MIN_SPEED) / (MAX_SPEED - MIN_SPEED);

      float distance = (e->body->GetPosition() - this->sun->body->GetPosition()).Length();
      if (distance > MIN_DISTANCE)
        vol = 0;
      else
        vol = vol * ((MIN_DISTANCE - distance) / MIN_DISTANCE);

      if (!mute)
        Mix_Volume(e->planetWhooshChannel, vol);
      else
        Mix_Volume(e->planetWhooshChannel, 0);
    }

  // Spawn new planet if needed.
  if (this->spawnPlanet) {
    this->SpawnPlanet();
    this->spawnPlanet = false;
  }

  // Advance physics.
  this->physicsTimeAccumulator += dt;
  while (this->physicsTimeAccumulator >= Config::PhysicsTimeStep) {
    // Update score.
    for (auto e : this->entities)
      if (e->isPlanet) {
        float v = e->body->GetLinearVelocityFromWorldPoint(e->body->GetPosition()).Length();
        float d = (e->body->GetPosition() - this->sun->body->GetPosition()).Length();
        float diff = v / d;
        if (d > 100) d = 0.0;
        this->scoreAccumulator += diff * 50 * Config::PhysicsTimeStep;
        if (this->scoreAccumulator >= 100) {
          this->SetScore(this->score + 100);
          this->scoreAccumulator -= 100;
          PlaySound("score-tik");
        }
      }

    // Apply forces.
    b2Vec2 gravity;
    for (auto e : this->entities) {
      if (e->isAffectedByGravity) {
        gravity.Set(0.0, 0.0);
        for (auto s : this->entities) {
          if (s->hasGravity) {
            b2Vec2 n = s->body->GetPosition() - e->body->GetPosition();
            float r2 = n.LengthSquared();
            n.Normalize();
            gravity += s->gravityCoeff / r2 * n;
          }
        }

        e->body->ApplyForce(gravity, e->body->GetWorldCenter(), true);
      }
    }

    this->world.Step(Config::PhysicsTimeStep, 10, 10);
    this->time += Config::PhysicsTimeStep;

    this->FixCamera();

    // Remove out of bounds planets
    int winw, winh;
    SDL_GetWindowSize(this->window, &winw, &winh);
    float width = winw / this->camera.ppm;
    float height = winh / this->camera.ppm;
    float minx = this->camera.pos.x;
    float maxx = this->camera.pos.x + width;
    float miny = this->camera.pos.y;
    float maxy = this->camera.pos.y + height;

    for (auto e : this->entities)
      if (e->isPlanet) {
        b2Vec2 pos = e->body->GetPosition();
        float r = e->body->GetFixtureList()->GetShape()->m_radius;

        //if (pos.x + r <= maxx && pos.x - r && minx && pos.y + r <= maxy && pos.y - r >= miny)
        //  continue;

        if ((pos.x + r >= minx && pos.x + r <= maxx && pos.y + r >= miny && pos.y + r <= maxy) ||
            (pos.x - r >= minx && pos.x - r <= maxx && pos.y + r >= miny && pos.y + r <= maxy) ||
            (pos.x - r >= minx && pos.x - r <= maxx && pos.y - r >= miny && pos.y - r <= maxy) ||
            (pos.x + r >= minx && pos.x + r <= maxx && pos.y - r >= miny && pos.y - r <= maxy))
          continue;

        bool trailPointVisible = false;
        for (auto &tp : e->trail.points) {
          if ((tp.pos.x + r >= minx && tp.pos.x + r <= maxx && tp.pos.y + r >= miny && tp.pos.y + r <= maxy) ||
              (tp.pos.x - r >= minx && tp.pos.x - r <= maxx && tp.pos.y + r >= miny && tp.pos.y + r <= maxy) ||
              (tp.pos.x - r >= minx && tp.pos.x - r <= maxx && tp.pos.y - r >= miny && tp.pos.y - r <= maxy) ||
              (tp.pos.x + r >= minx && tp.pos.x + r <= maxx && tp.pos.y - r >= miny && tp.pos.y - r <= maxy))
            trailPointVisible = true;
          break;
        }
        if (trailPointVisible || e->trail.points.size() == 0)
          continue;

        this->DiscardPlanet(e);
      }

    // Remove and properly destroy entities marked to be removed.
    for (auto e : this->toBeRemoved) {
      if (e->hasPhysics)
        this->world.DestroyBody(e->body);
      delete e;
      this->entities.erase(find(this->entities.begin(),
                                this->entities.end(),
                                e));
    }

    // Update the trails.
    UpdateTrails();

    this->toBeRemoved.clear();
    this->physicsTimeAccumulator -= Config::PhysicsTimeStep;
  }

  this->stepOnce = false;
}

void GameScreen::Render(Renderer *renderer) {
  renderer->SetCamera(this->camera);

  this->background.Draw();
  //this->DrawGrid(renderer);

  for (auto e : this->entities)
    if (e->hasTrail)
      this->DrawTrail(renderer, e);

  for (auto e : this->entities)
    if (e->isDrawable)
      e->mesh->Draw(e->body->GetPosition(), e->body->GetAngle());

  // Count this frame.
  if (!this->paused)
    this->frameCount++;

  for (auto w : this->widgets)
    w->Render(renderer);

  renderer->PresentScreen();
}

void GameScreen::FixCamera() {
  for (auto e : this->entities)
    if (e->isPlanet)
      this->FixCamera(e);
}

void GameScreen::FixCamera(Entity *e) {
  int winw, winh;
  SDL_GetWindowSize(this->window, &winw, &winh);
  float ratio = ((float) winw) / winh;

  float width, height;

  b2Vec2 upper(this->camera.pos.x + winw / this->camera.ppm,
               this->camera.pos.y + winh / this->camera.ppm);
  b2Vec2 lower(this->camera.pos.x, this->camera.pos.y);

  auto r = e->body->GetFixtureList()->GetShape()->m_radius;

  auto pos = e->body->GetPosition();

  float maxx, maxy, minx, miny;

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

  auto halfx = max(fabs(maxx), fabs(minx));
  auto halfy = max(fabs(maxy), fabs(miny));

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

  if (width > Config::CameraMaxWidth) {
    width = Config::CameraMaxWidth;
    height = width / ratio;
  }
  if (width < Config::CameraMinWidth) {
    width = Config::CameraMinWidth;
    height = width / ratio;
  }
  if (height > Config::CameraMaxHeight) {
    height = Config::CameraMaxHeight;
    width = height * ratio;
  }
  if (height < Config::CameraMinHeight) {
    height = Config::CameraMinHeight;
    width = height * ratio;
  }

  this->camera.pos.x = - (width / 2.0);
  this->camera.pos.y = - (height / 2.0);

  this->camera.ppm = winw / width;

  auto program = ResourceCache::texturedPolygonProgram;

  glUseProgram(program);

  GLuint cameraPosUniform = glGetUniformLocation(program, "camera_pos");
  GLuint ppmUniform = glGetUniformLocation(program, "ppm");

  glUniform2f(cameraPosUniform, this->camera.pos.x, this->camera.pos.y);
  glUniform1f(ppmUniform, this->camera.ppm);

  glUseProgram(0);
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
      e->trail.points.push_back(TrailPoint(e->body->GetPosition(), this->time));
    }
}

void GameScreen::AddRandomCollectible() {
  // Choose a random position, but make sure it is not too close to
  // another collectible.
  b2Vec2 pos;
  bool retry;
  do {
    retry = false;
    pos = this->GetRandomPosition();
    for (auto e : this->entities)
      if (e->isCollectible) {
        float distanceSq = (e->body->GetPosition() - pos).LengthSquared();
        if (distanceSq < 25.0)
          retry = true;
      }
  } while (retry);

  CollectibleType types[] = {CollectibleType::PLUS_SCORE,
                             CollectibleType::MINUS_SCORE,
                             CollectibleType::PLUS_TIME,
                             CollectibleType::MINUS_TIME,
                             CollectibleType::SPAWN_PLANET};
  CollectibleType type = types[rand() % (sizeof(types) / sizeof(types[0]))];
  this->entities.push_back(Entity::CreateCollectible(&this->world,
                                                     pos,
                                                     type));
}

void GameScreen::AddRandomEnemy() {
  int winw, winh;
  SDL_GetWindowSize(this->window, &winw, &winh);

  float dx = frand() * 2.0;
  float dy = frand() * 2.0;

  float x1 = this->camera.pos.x;
  float y1 = this->camera.pos.y;
  float x2 = this->camera.pos.x + winw / this->camera.ppm;
  float y2 = this->camera.pos.y + winh / this->camera.ppm;
  float w = x2 - x1;
  float h = y2 - y1;

  float randomx = x1 + w * frand();
  float randomy = y1 + h * frand();

  b2Vec2 pos;
  int r = rand() % 4;
  if (r == 0)
    pos.Set(randomx, y1 - dy);
  else if (r == 1)
    pos.Set(randomx, y2 + dy);
  else if (r == 2)
    pos.Set(x1 - dx, randomy);
  else
    pos.Set(x2 + dx, randomy);

  b2Vec2 v = this->sun->body->GetPosition() - pos;
  v.Normalize();
  v *= 20.0;

  float angle = atan2(v.y, v.x) - M_PI / 2.0;
  this->entities.push_back(Entity::CreateEnemyShip(&this->world,
                                                   pos,
                                                   v,
                                                   angle));
}

void GameScreen::TimerCallback(float elapsed) {
  if (this->gameOverLabel->GetVisible()) {
    this->state["name"] = "game-over";
    this->state["score"] = to_string(this->score);
    return;
  }

  // Remove out of bounds enemy ships.
  for (auto e : this->entities)
    if (e->isEnemy)
      if (e->body->GetPosition().LengthSquared() > pow(Config::CameraMaxWidth / 2.0, 2) + pow(Config::CameraMaxHeight / 2.0, 2) + 25.0)
        this->toBeRemoved.push_back(e);

  // Update FPS counter.
  this->fps = this->frameCount;
#ifndef RELEASE_BUILD
    stringstream ss;
    ss << "FPS: " << this->fps;
    this->fpsLabel->SetText(ss.str());
#endif
  this->frameCount = 0;

  // Decrement remaining time.
  if (this->timeRemaining > 0)
    this->SetTimeRemaining(this->timeRemaining - 1);

  // Occasionally add collectibles.
  if (rand() % 8 == 0)
    this->AddRandomCollectible();

  // Occasionally add enemy ships.
  int n;
  if (this->time < 30)
    n = 20;
  else if (this->time < 60)
    n = 15;
  else if (this->time < 90)
    n = 10;
  else
    n = 5;
  if (rand() % n == 0)
    this->AddRandomEnemy();
}

void GameScreen::DrawGrid(Renderer *renderer) const {
  // Draw grid.
  /*int winw, winh;
  SDL_GetWindowSize(this->window, &winw, &winh);

  float upperx = this->camera.pos.x + winw / this->camera.ppm;
  float uppery = this->camera.pos.y + winh / this->camera.ppm;
  float x = this->camera.pos.x + fmod(this->camera.pos.x + 10, 10);
  float y = this->camera.pos.x + fmod(this->camera.pos.y + 10, 10);
  for (; x <= upperx; x += 10)
    renderer->DrawLine(b2Vec2(x, this->camera.pos.y), b2Vec2(x, uppery), 32, 32, 32, 255);
  for (; y <= uppery; y += 10)
  renderer->DrawLine(b2Vec2(this->camera.pos.x, y), b2Vec2(upperx, y), 32, 32, 32, 255);*/
}

void GameScreen::DrawTrail(Renderer *renderer, const Entity *e) const {
  vector<TrailPoint> points;

  if (!e->hasTrail)
    return;

  if (e->trail.size < e->trail.points.size()) {
    // Choose 'e->trail.size' points in the 'e->trail.time' time-window.

    // From the rest choose enough, as evenly timed as possible.
    auto step = e->trail.time / e->trail.size;
    auto time = e->trail.points.back().time;
    auto it = e->trail.points.rbegin();
    while (points.size() < e->trail.size) {
      time -= step;

      // Go forward among previous locations until we reach one after
      // 'time'. Choose the point as close to the time we want as
      // possible.
      float leastDiff = FLT_MAX;
      TrailPoint closestPoint;
      for (; it != e->trail.points.rend(); ++it) {
        if (fabs(it->time - time) < leastDiff) {
          leastDiff = fabs(it->time - time);
          closestPoint = *it;
        }

        if (it->time <= time)
          break;
      }

      points.push_back(closestPoint);

      // Continue from this point.
      time = closestPoint.time;
    }

    // We have chosen the trail points from the last to the first, so
    // reverse them.
    std::reverse(points.begin(), points.end());
  }
  else
    points = e->trail.points;

  float r = e->body->GetFixtureList()->GetShape()->m_radius;
  auto startr = r / 10.0;
  auto endr = r / 2.0;
  r = startr;

  const float starta = 0.25;
  const float enda = 0.5;
  float a = starta;

  float dr, da;
  if (points.size() > 0) {
    dr = (endr - startr) / points.size();
    da = (enda - starta) / points.size();
  }

  for (auto &p : points) {
    float scale_factor = r / e->body->GetFixtureList()->GetShape()->m_radius;
    this->trailPointMesh->SetColor(1.0, 1.0, 1.0, a);
    this->trailPointMesh->Draw(p.pos, 0.0f, scale_factor);
    r += dr;
    a += da;
  }
}
