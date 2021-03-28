#ifndef _GRAVITY_ENTITY_HH_
#define _GRAVITY_ENTITY_HH_

#include "mesh.hh"

#include <box2d/box2d.h>

#include <vector>
#include <ostream>

using namespace std;

struct TrailPoint {
  TrailPoint() :
    time(0.0)
  {}

  TrailPoint(b2Vec2 pos, float time) :
    pos(pos),
    time(time)
  {}

  b2Vec2 pos;
  float time;
};

struct Trail {
  Trail() :
    size(0),
    time(0.0)
  {}

  int size;
  float time;
  vector<TrailPoint> points;
};

enum class CollectibleType {
  PLUS_SCORE,
  MINUS_SCORE,
  PLUS_TIME,
  MINUS_TIME,
  SPAWN_PLANET,
};

struct Entity {
protected:
  void SaveBody(const b2Body *b, ostream &s) const;
  void SaveTrail(const Trail &t, ostream &s) const;
  b2Body *LoadBody(istream &s, b2World *world);
  Trail LoadTrail(istream &s);

public:
  Entity();
  ~Entity();

  bool hasPhysics;
  b2Body *body;

  bool hasGravity;
  float gravityCoeff;

  bool hasTrail;
  Trail trail;

  bool isAffectedByGravity;
  bool isSun;
  bool isEnemy;

  bool isPlanet;
  int planetWhooshChannel;

  bool isCollectible;
  bool hasScore;
  int score;
  bool hasTime;
  int time;
  bool spawnPlanet;

  bool isDrawable;
  Mesh *mesh;

  void Save(ostream &s) const;
  void Load(istream &s, b2World *world);

  static Entity *CreatePlanet(b2World *world,
                              b2Vec2 pos,
                              float radius,
                              float density,
                              b2Vec2 v0=b2Vec2(0, 0));
  static Entity *CreateSun(b2World *world,
                           b2Vec2 pos,
                           float radius,
                           float density,
                           float gravityCoeff);
  static Entity *CreateCollectible(b2World *world,
                                   b2Vec2 pos,
                                   CollectibleType type);
  static Entity *CreateEnemyShip(b2World *world,
                                 b2Vec2 pos,
                                 b2Vec2 velocity,
                                 float angle);
};

#endif /* _GRAVITY_ENTITY_HH_ */
