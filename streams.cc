#include <Box2D/Box2D.h>

#include <iostream>

using namespace std;

ostream &operator<<(ostream &s, const b2Vec2 &v) {
  return s << v.x << v.y;
}

istream &operator>>(istream &s, b2Vec2 &v) {
  return s >> v.x >> v.y;
}
