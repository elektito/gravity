#ifndef _GRAVITY_GAME_SCREEN_HH_
#define _GRAVITY_GAME_SCREEN_HH_

#include "screen.hh"
#include "camera.hh"
#include "timer.hh"

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
};

class GameScreen : public Screen {
protected:
  // state variables
  int score;
  int timeRemaining;
  bool paused;
  Camera camera;
  Trail trail;
  float32 startTime;

  // non-state variables
  b2World world;
  b2Body *draggingBody;
  b2Vec2 draggingOffset;
  bool stepOnce;
  Timer timer;
  ContactListener contactListener;

  // methods
  void FixCamera(b2Body *body);
  void TimerCallback(float elapsed);
  void UpdateTrail(b2Body *b, Trail *t, float32 currentTime);

  friend class ContactListener;

public:
  GameScreen(SDL_Window *window);
  virtual ~GameScreen();

  virtual void HandleEvent(const SDL_Event &e);

  virtual void Reset();
  virtual void Save(ostream &s) const;
  virtual void Load(istream &s);

  virtual void Advance(float dt);
  virtual void Render(Renderer *renderer) const;
};

#endif /* _GRAVITY_GAME_SCREEN_HH_ */
