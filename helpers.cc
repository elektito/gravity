#include "helpers.hh"
#include "font-cache.hh"

#include <Box2D/Box2D.h>

#include <iostream>
#include <map>

using namespace std;

void SaveVec2(const b2Vec2 &v, ostream &s) {
  WRITE(v.x, s);
  WRITE(v.y, s);
}

void LoadVec2(b2Vec2 &v, istream &s) {
  READ(v.x, s);
  READ(v.y, s);
}

void SaveString(const string &str, ostream &s) {
  size_t size = str.size();
  WRITE(size, s);
  s.write((const char*) str.data(), size);
}

void LoadString(string &str, istream &s) {
  size_t size;
  READ(size, s);
  str.resize(size);
  s.read((char*) str.data(), size);
}

void SaveMap(const map<string, string> &m, ostream &s) {
  size_t size = m.size();
  WRITE(size, s);
  for (auto &p : m) {
    SaveString(p.first, s);
    SaveString(p.second, s);
  }
}

void LoadMap(map<string, string> &m, istream &s) {
  string k, v;
  size_t n;

  m.clear();

  READ(n, s);
  for (int i = 0; i < n; ++i) {
    LoadString(k, s);
    LoadString(v, s);
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
