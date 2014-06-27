#include "font-cache.hh"

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

void GetTextWidthP(string text, float32 hp, SDL_Window *window, float32 &wp) {
  int winw, winh;
  SDL_GetWindowSize(window, &winw, &winh);

  int height_pixels = hp * winh;
  TTF_Font *font = FontCache::GetFont(height_pixels);

  int w, h;
  TTF_SizeText(font, text.data(), &w, &h);
  wp = (float32) w / winw;
}
