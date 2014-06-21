#include <Box2D/Box2D.h>

#include <iostream>
#include <map>

using namespace std;

ostream &operator<<(ostream &s, const b2Vec2 &v) {
  return s << v.x << v.y;
}

istream &operator>>(istream &s, b2Vec2 &v) {
  return s >> v.x >> v.y;
}

void SaveMap(const map<string, string> &m, ostream &s) {
  s << m.size();
  for (auto &p : m) {
    s << p.first;
    s << p.second;
  }
}

void LoadMap(map<string, string> &m, istream &s) {
  string k, v;
  size_t n;

  m.clear();

  s >> n;
  for (int i = 0; i < n; ++i) {
    s >> k >> v;
    m[k] = v;
  }
}
