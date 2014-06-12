#ifndef _GRAVITY_STREAMS_HH_
#define _GRAVITY_STREAMS_HH_

#include <Box2D/Box2D.h>

#include <iostream>

using namespace std;

extern ostream &operator<<(ostream &s, const b2Vec2 &v);
extern istream &operator>>(istream &s, b2Vec2 &v);

#endif /* _GRAVITY_STREAMS_HH_ */
