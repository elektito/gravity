#include "entity.hh"

Entity *Entity::CreatePlanet(b2World *world,
                             b2Vec2 pos,
                             float32 radius,
                             float32 density)
{
  Entity *e = new Entity;
  e->hasPhysics = true;
  b2BodyDef bd;
  bd.type = b2_dynamicBody;
  bd.position = pos;
  e->body = world->CreateBody(&bd);

  b2CircleShape shape;
  shape.m_p.Set(0.0, 0.0);
  shape.m_radius = radius;

  b2FixtureDef fd;
  fd.shape = &shape;
  fd.friction = 0.5;
  fd.restitution = 0.7;
  fd.density = density;
  e->body->CreateFixture(&fd);

  e->hasTrail = true;
  e->trail.size = 30;
  e->trail.time = 1.0;

  e->hasGravity = false;
  e->isAffectedByGravity = true;
  e->body->SetUserData(e);

  return e;
}

Entity *Entity::CreateSun(b2World *world,
                          b2Vec2 pos,
                          float32 radius,
                          float32 density,
                          float32 gravityCoeff)
{
  Entity *e = new Entity;
  e->hasPhysics = true;
  b2BodyDef bd;
  bd.type = b2_dynamicBody;
  bd.position = pos;
  e->body = world->CreateBody(&bd);

  b2CircleShape shape;
  shape.m_p.Set(0.0, 0.0);
  shape.m_radius = radius;

  b2FixtureDef fd;
  fd.shape = &shape;
  fd.friction = 0.5;
  fd.restitution = 0.7;
  fd.density = density;
  e->body->CreateFixture(&fd);

  e->hasGravity = true;
  e->gravityCoeff = gravityCoeff;
  e->isAffectedByGravity = false;
  e->body->SetUserData(e);

  return e;
}
