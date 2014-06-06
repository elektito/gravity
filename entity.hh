#ifndef _GRAVITY_ENTITY_HH_
#define _GRAVITY_ENTITY_HH_

#include <Box2D/Box2D.h>

struct Entity {
  b2Body *body;

  bool isGravitySource;
  float32 gravityCoeff;
};

#endif /* _GRAVITY_ENTITY_HH_ */
