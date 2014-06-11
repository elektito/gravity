#ifndef _GRAVITY_ENTITY_HH_
#define _GRAVITY_ENTITY_HH_

#include <Box2D/Box2D.h>

#include <vector>

using namespace std;

struct TrailPoint {
  b2Vec2 pos;
  float32 time;
};

struct Trail {
  int size;
  float32 time;
  vector<TrailPoint> points;
};

struct Entity {
  bool hasPhysics;
  b2Body *body;

  bool hasGravity;
  float32 gravityCoeff;

  bool hasTrail;
  Trail trail;

  bool isAffectedByGravity;
  bool isSun;
  bool isPlanet;

  static Entity *CreatePlanet(b2World *world,
                              b2Vec2 pos,
                              float32 radius,
                              float32 density);
  static Entity *CreateSun(b2World *world,
                           b2Vec2 pos,
                           float32 radius,
                           float32 density,
                           float32 gravityCoeff);
};

#endif /* _GRAVITY_ENTITY_HH_ */
