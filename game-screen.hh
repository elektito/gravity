#ifndef _GRAVITY_GAME_SCREEN_HH_
#define _GRAVITY_GAME_SCREEN_HH_

#include "screen.hh"
#include "camera.hh"
#include "timer.hh"
#include "entity.hh"
#include "label-widget.hh"
#include "number-widget.hh"
#include "image-button-widget.hh"
#include "mesh.hh"

#include <box2d/box2d.h>

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
  float time;
  int score;
  int timeRemaining;
  bool paused;
  Camera camera;
  float physicsTimeAccumulator;
  float scoreAccumulator;
  int lives;
  bool spawnPlanet;
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
  Background background;
  bool mouseDown;
  int mouseDownX;
  int mouseDownY;
  bool discardLeftButtonUp;

  NumberWidget *scoreLabel;
  LabelWidget *timeLabel;
#ifndef RELEASE_BUILD
  LabelWidget *fpsLabel;
#endif
  ImageWidget *continueLabel;
  ImageWidget *pauseSign;
  ImageButtonWidget *endGameButton;
  ImageButtonWidget *muteButton;
  ImageWidget *gameOverLabel;
  ImageWidget *livesLabel;

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
  void TogglePause();
  void DecreaseLives();
  void DiscardPlanet(Entity *planet);

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
