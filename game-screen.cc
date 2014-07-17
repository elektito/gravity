#include "game-screen.hh"
#include "entity.hh"
#include "helpers.hh"
#include "resource-cache.hh"
#include "config.hh"

#include <sstream>
#include <iomanip>
#include <functional>

using std::placeholders::_1;

using namespace std;

b2Vec2 RotatePoint(b2Vec2 point, float32 theta, b2Vec2 center) {
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
  Entity *e1 = (Entity*) contact->GetFixtureA()->GetBody()->GetUserData();
  Entity *e2 = (Entity*) contact->GetFixtureB()->GetBody()->GetUserData();

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
  Mix_PlayChannel(-1, ResourceCache::GetSound("enemy-collision"), 0);

  this->screen->timeRemaining -= 10;
  if (this->screen->timeRemaining < 0)
    this->screen->timeRemaining = 0;

  int minutes = this->screen->timeRemaining / 60;
  int seconds = this->screen->timeRemaining % 60;
  stringstream ss;
  ss << setw(2) << setfill('0') << minutes
     << setw(0) << ":"
     << setw(2) << setfill('0') << seconds;
  this->screen->timeLabel->SetText(ss.str());

  this->screen->toBeRemoved.push_back(enemy);
}

void ContactListener::EnemyPlanetContact(Entity *enemy, Entity *sun) {
  Mix_PlayChannel(-1, ResourceCache::GetSound("enemy-collision"), 0);

  this->screen->timeRemaining -= 10;
  if (this->screen->timeRemaining < 0)
    this->screen->timeRemaining = 0;

  int minutes = this->screen->timeRemaining / 60;
  int seconds = this->screen->timeRemaining % 60;
  stringstream ss;
  ss << setw(2) << setfill('0') << minutes
     << setw(0) << ":"
     << setw(2) << setfill('0') << seconds;
  this->screen->timeLabel->SetText(ss.str());

  this->screen->toBeRemoved.push_back(enemy);
}

void ContactListener::PlanetSunContact(Entity *planet, Entity *sun) {
  Mix_PlayChannel(-1, ResourceCache::GetSound("planet-sun-collision"), 0);

  if (!this->inContact)
    this->screen->timeRemaining -= 10;
  if (this->screen->timeRemaining < 0)
    this->screen->timeRemaining = 0;

  int minutes = this->screen->timeRemaining / 60;
  int seconds = this->screen->timeRemaining % 60;
  stringstream ss;
  ss << setw(2) << setfill('0') << minutes
     << setw(0) << ":"
     << setw(2) << setfill('0') << seconds;
  this->screen->timeLabel->SetText(ss.str());

  this->inContact = true;
}

void ContactListener::CollectibleSunContact(Entity *collectible, Entity *sun) {
  Mix_PlayChannel(-1, ResourceCache::GetSound("sun-powerup"), 0);

  if (collectible->hasScore)
    this->screen->score += collectible->score;

  this->screen->toBeRemoved.push_back(collectible);
}

void ContactListener::CollectiblePlanetContact(Entity *collectible, Entity *planet) {
  Mix_PlayChannel(-1, ResourceCache::GetSound("planet-powerup"), 0);

  if (collectible->hasScore)
    this->screen->score += 10 * collectible->score;

  this->screen->toBeRemoved.push_back(collectible);
}

void ContactListener::EndContact(b2Contact *contact) {
  this->inContact = false;
}

bool ContactFilter::ShouldCollide(b2Fixture *fixtureA, b2Fixture *fixtureB) {
  Entity *e1 = (Entity*) fixtureA->GetBody()->GetUserData();
  Entity *e2 = (Entity*) fixtureB->GetBody()->GetUserData();

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

float32 frand() {
  return rand() / (float32) RAND_MAX;
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

  this->world.SetContactListener(&this->contactListener);
  this->world.SetContactFilter(&this->contactFilter);

  this->scoreLabel = new LabelWidget(this,
                                    "000000",
                                    0.02, 0.0, 0.1,
                                    TextAnchor::RIGHT, TextAnchor::TOP,
                                    {255, 255, 255, 128});

  this->timeLabel = new LabelWidget(this,
                                     "02:00",
                                     0.02, 0.0, 0.1,
                                     TextAnchor::LEFT, TextAnchor::TOP,
                                     {255, 255, 255, 128});
  this->fpsLabel = new LabelWidget(this,
                                   "FPS: 0",
                                   0.02, 0.0, 0.1,
                                   TextAnchor::LEFT, TextAnchor::BOTTOM,
                                   {255, 255, 255, 128});
  this->continueLabel = new LabelWidget(this,
                                        "[Press P to continue]",
                                        0.0, 0.02, 0.05,
                                        TextAnchor::CENTER, TextAnchor::BOTTOM,
                                        {255, 255, 255, 128});
  this->endGameButton = new ButtonWidget(this,
                                         "End Game",
                                         0.02, 0.02, 0.05,
                                         TextAnchor::RIGHT, TextAnchor::BOTTOM,
                                         {255, 0, 0, 128},
                                         {255, 255, 255, 128});

  this->widgets.push_back(this->scoreLabel);
  this->widgets.push_back(this->timeLabel);
  this->widgets.push_back(this->fpsLabel);
  this->widgets.push_back(this->continueLabel);
  this->widgets.push_back(this->endGameButton);

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

  // Create the "pause sign" mesh.
  GLfloat pauseVertexData[] = {
    // triangle 1
    /* coord */ -0.3f, -0.3f, /* tex_coord */ 0.0f, 0.0f,
    /* coord */ -0.3f,  0.3f, /* tex_coord */ 0.0f, 1.0f,
    /* coord */  0.3f, -0.3f, /* tex_coord */ 1.0f, 0.0f,

    // triangle 2
    /* coord */ -0.3f,  0.3f, /* tex_coord */ 0.0f, 1.0f,
    /* coord */  0.3f,  0.3f, /* tex_coord */ 1.0f, 1.0f,
    /* coord */  0.3f, -0.3f, /* tex_coord */ 1.0f, 0.0f,
  };

  this->pauseMesh = new Mesh(pauseVertexData, 6, ResourceCache::GetTexture("pause"), true);

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

void GameScreen::SwitchScreen(const map<string, string> &lastState) {

}

void GameScreen::HandleEvent(const SDL_Event &e) {
  int x, y;

  switch (e.type) {
  case SDL_MOUSEBUTTONDOWN:
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
    }
    break;

  case SDL_KEYDOWN:
    switch (e.key.keysym.sym) {
    case SDLK_p:
      this->paused = !this->paused;
      this->fpsLabel->SetVisible(!this->paused);
      this->continueLabel->SetVisible(this->paused);
      this->endGameButton->SetVisible(this->paused);
      Timer::TogglePauseAll();
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

  for (auto w : this->widgets)
    w->HandleEvent(e);
}

void GameScreen::HandleWidgetEvent(int event_type, Widget *widget) {
  switch (event_type) {
  case BUTTON_CLICK:
    if (widget == this->endGameButton) { // End Game
      this->state["name"] = "game-over";
      this->state["score"] = to_string(this->score);
    }

    break;
  } // switch (event_type)
}

void GameScreen::Reset() {
  this->state.clear();
  this->state["name"] = "playing";

  this->score = 0;
    this->scoreLabel->SetText("000000");
  this->timeRemaining = Config::GameTime;
    int minutes = this->timeRemaining / 60;
    int seconds = this->timeRemaining % 60;
    stringstream ss;
    ss << setw(2) << setfill('0') << minutes
       << setw(0) << ":"
       << setw(2) << setfill('0') << seconds;
    this->timeLabel->SetText(ss.str());
  this->paused = true;
  this->time = 0.0;

  this->camera.pos.Set(-50.0, -50.0);
  this->camera.ppm = 10.0;

  this->draggingBody = nullptr;
  this->stepOnce = false;
  this->physicsTimeAccumulator = 0.0;
  this->scoreAccumulator = 0;

  // Remove existing entities.
  for (auto e : this->entities) {
    if (e->hasPhysics)
      this->world.DestroyBody(e->body);
    delete e;
  }
  this->entities.clear();

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

  this->fpsLabel->SetVisible(!this->paused);
  this->continueLabel->SetVisible(this->paused);
  this->endGameButton->SetVisible(this->paused);
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

  size_t size = this->entities.size();
  WRITE(size, s);
  for (auto e : this->entities)
    e->Save(s);
}

void GameScreen::Load(istream &s) {
  LoadMap(this->state, s);

  READ(this->time, s);
  READ(this->score, s);
    stringstream ss;
    ss << setw(6) << setfill('0') << this->score;
    this->scoreLabel->SetText(ss.str());
  READ(this->timeRemaining, s);
    int minutes = this->timeRemaining / 60;
    int seconds = this->timeRemaining % 60;
    stringstream ss2;
    ss2 << setw(2) << setfill('0') << minutes
        << setw(0) << ":"
        << setw(2) << setfill('0') << seconds;
    this->timeLabel->SetText(ss2.str());
  READ(this->paused, s);
  READ(this->camera.pos.x, s);
  READ(this->camera.pos.y, s);
  READ(this->camera.ppm, s);
  READ(this->physicsTimeAccumulator, s);
  READ(this->scoreAccumulator, s);

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
  for (auto w : this->widgets)
    w->Advance(dt);

  if (this->state["name"] == "game-over")
    return;

  if (this->paused && !this->stepOnce)
    return;

  Timer::CheckAll();

  // Advance physics.
  this->physicsTimeAccumulator += dt;
  while (this->physicsTimeAccumulator >= Config::PhysicsTimeStep) {
    // Update score.
    for (auto e : this->entities)
      if (e->isPlanet) {
        float32 v = e->body->GetLinearVelocityFromWorldPoint(e->body->GetPosition()).Length();
        float32 d = (e->body->GetPosition() - this->sun->body->GetPosition()).Length();
        float32 diff = v / d;
        if (d > 100) d = 0.0;
        this->scoreAccumulator += diff * 50 * Config::PhysicsTimeStep;
        if (this->scoreAccumulator >= 100) {
          this->score += 100;
            stringstream ss;
            ss << setw(6) << setfill('0') << this->score;
            this->scoreLabel->SetText(ss.str());
          this->scoreAccumulator -= 100;
          Mix_PlayChannel(-1, ResourceCache::GetSound("score-tik"), 0);
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
            float32 r2 = n.LengthSquared();
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

    // Remove and properly destroy entities marked to be removed.
    for (auto e : this->toBeRemoved) {
      this->world.DestroyBody(e->body);
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

  renderer->DrawBackground();
  //this->DrawGrid(renderer);

  for (auto e : this->entities)
    if (e->hasTrail)
      this->DrawTrail(renderer, e);

  for (auto e : this->entities)
    if (e->isDrawable)
      e->mesh->Draw(e->body->GetPosition(), e->body->GetAngle());

  // Draw HUD.
  if (this->paused)
    this->pauseMesh->Draw(b2Vec2(0.0f, 0.0f), 0.0f);

  // Draw origin.
  //renderer->DrawLine(b2Vec2(0, 1), b2Vec2(0, -1), 255, 0, 0, 255);
  //renderer->DrawLine(b2Vec2(1, 0), b2Vec2(-1, 0), 255, 0, 0, 255);

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
  const float32 MIN_DISTANCE = 5;

  int wpixels, hpixels;
  SDL_GetWindowSize(this->window, &wpixels, &hpixels);

  // Get window dimensions in meters.
  float32 width = wpixels / this->camera.ppm;
  float32 height = hpixels / this->camera.ppm;

  b2Vec2 pos;
  while (true) {
    // Choose a random position for the collectible.
    pos.x = this->camera.pos.x + (float32) rand() / RAND_MAX * width;
    pos.y = this->camera.pos.y + (float32) rand() / RAND_MAX * height;

    // Retry if the chosen position is to close to a sun or a planet.
    for (auto e : this->entities)
      if (e->isSun || e->isPlanet) {
        // Get sun/planet radius.
        float32 r = e->body->GetFixtureList()->GetShape()->m_radius;

        // If distance from the surface (indicated by the '+r') is
        // less than minimum distance, retry.
        if ((e->body->GetPosition() - pos).Length() + r < MIN_DISTANCE);
          continue;
      }

    break;
  }
  this->entities.push_back(Entity::CreateScoreCollectible(&this->world,
                                                          pos));
}

void GameScreen::AddRandomEnemy() {
  int winw, winh;
  SDL_GetWindowSize(this->window, &winw, &winh);

  float32 dx = frand() * 2.0;
  float32 dy = frand() * 2.0;

  float32 x1 = this->camera.pos.x;
  float32 y1 = this->camera.pos.y;
  float32 x2 = this->camera.pos.x + winw / this->camera.ppm;
  float32 y2 = this->camera.pos.y + winh / this->camera.ppm;
  float32 w = x2 - x1;
  float32 h = y2 - y1;

  float32 randomx = x1 + w * frand();
  float32 randomy = y1 + h * frand();

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

  float32 angle = atan2(v.y, v.x) - M_PI / 2.0;
  this->entities.push_back(Entity::CreateEnemyShip(&this->world,
                                                   pos,
                                                   v,
                                                   angle));
}

void GameScreen::TimerCallback(float elapsed) {
  // Update FPS counter.
  this->fps = this->frameCount;
    stringstream ss;
    ss << "FPS: " << this->fps;
    this->fpsLabel->SetText(ss.str());
  this->frameCount = 0;

  // Decrement remaining time.
  if (this->timeRemaining > 0) {
    this->timeRemaining--;

    int minutes = this->timeRemaining / 60;
    int seconds = this->timeRemaining % 60;
    stringstream ss;
    ss << setw(2) << setfill('0') << minutes
       << setw(0) << ":"
       << setw(2) << setfill('0') << seconds;
    this->timeLabel->SetText(ss.str());
  }

  // Check for game over.
  if (this->timeRemaining == 0) {
    this->state["name"] = "game-over";
    this->state["score"] = to_string(this->score);
    return;
  }

  // Occasionally add collectibles.
  if (rand() % 5 == 0)
    this->AddRandomCollectible();

  // Occasionally add enemy ships.
  if (rand() % 10 == 0)
    this->AddRandomEnemy();
}

void GameScreen::DrawGrid(Renderer *renderer) const {
  // Draw grid.
  /*int winw, winh;
  SDL_GetWindowSize(this->window, &winw, &winh);

  float32 upperx = this->camera.pos.x + winw / this->camera.ppm;
  float32 uppery = this->camera.pos.y + winh / this->camera.ppm;
  float32 x = this->camera.pos.x + fmod(this->camera.pos.x + 10, 10);
  float32 y = this->camera.pos.x + fmod(this->camera.pos.y + 10, 10);
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
      float32 leastDiff = FLT_MAX;
      TrailPoint closestPoint;
      for (; it != e->trail.points.rend(); ++it) {
        if (abs(it->time - time) < leastDiff) {
          leastDiff = abs(it->time - time);
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

  float32 r = e->body->GetFixtureList()->GetShape()->m_radius;
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
    float scale_factor = r / e->body->GetFixtureList()->GetShape()->m_radius;
    this->trailPointMesh->Draw(p.pos, 0.0f, scale_factor);
    r += dr;
    a += da;
  }
}
