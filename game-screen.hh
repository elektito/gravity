#ifndef _GRAVITY_GAME_SCREEN_HH_
#define _GRAVITY_GAME_SCREEN_HH_

#include "screen.hh"
#include "camera.hh"
#include "timer.hh"
#include "entity.hh"
#include "label-widget.hh"
#include "button-widget.hh"
#include "mesh.hh"

#include <SDL2/SDL_mixer.h>
#include <Box2D/Box2D.h>

class GameScreen;

class ContactListener : public b2ContactListener {
protected:
  GameScreen *screen;
  bool inContact;

public:
  ContactListener(GameScreen *screen);

  virtual void BeginContact(b2Contact *contact);
  virtual void EndContact(b2Contact *contact);

  void EnemySunContact(Entity *enemy, Entity *sun);
  void EnemyPlanetContact(Entity *enemy, Entity *sun);
  void PlanetSunContact(Entity *planet, Entity *sun);
  void CollectibleSunContact(Entity *collectible, Entity *sun);
  void CollectiblePlanetContact(Entity *collectible, Entity *planet);

};

class ContactFilter : public b2ContactFilter {
public:
  bool ShouldCollide(b2Fixture *fixtureA, b2Fixture *fixtureB);
};

class GameScreen : public Screen {
protected:
  // state variables
  float32 time;
  int score;
  int timeRemaining;
  bool paused;
  Camera camera;
  float32 physicsTimeAccumulator;
  float32 scoreAccumulator;
  vector<Entity*> entities;

  // non-state variables
  b2World world;
  b2Body *draggingBody;
  b2Vec2 draggingOffset;
  bool stepOnce;
  Timer timer;
  ContactListener contactListener;
  ContactFilter contactFilter;
  Entity *sun;
  int frameCount;
  int fps;
  vector<Entity*> toBeRemoved;
  Mesh *trailPointMesh;
  Mesh *pauseMesh;
  bool spawnPlanet;
  Background background;

  LabelWidget *scoreLabel;
  LabelWidget *timeLabel;
  LabelWidget *fpsLabel;
  LabelWidget *continueLabel;
  ButtonWidget *endGameButton;

  // methods
  void FixCamera();
  void FixCamera(Entity *e);
  void TimerCallback(float elapsed);
  void UpdateTrails();
  void AddRandomCollectible();
  void AddRandomEnemy();
  void SetScore(int score);
  void SetTimeRemaining(int time);
  b2Vec2 GetRandomPosition();
  void SpawnPlanet();

  void DrawGrid(Renderer *renderer) const;
  void DrawTrail(Renderer *renderer, const Entity *entity) const;

  friend class ContactListener;

public:
  GameScreen(SDL_Window *window);
  virtual ~GameScreen();

  virtual void SwitchScreen(const map<string, string> &lastState);
  void HandleWidgetEvent(int event_type, Widget *widget);
  virtual void HandleEvent(const SDL_Event &e);

  virtual void Reset();
  virtual void Save(ostream &s) const;
  virtual void Load(istream &s);

  virtual void Advance(float dt);
  virtual void Render(Renderer *renderer);
};

#endif /* _GRAVITY_GAME_SCREEN_HH_ */
