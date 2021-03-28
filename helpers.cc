#include "helpers.hh"
#include "resource-cache.hh"

#include <box2d/box2d.h>

#include <iostream>
#include <map>
#include <fstream>
#include <sstream>

using namespace std;

bool mute = false;

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

void GetTextWidthP(string text, float hp, SDL_Window *window, float &wp) {
  int winw, winh;
  SDL_GetWindowSize(window, &winw, &winh);

  int height_pixels = hp * winh;
  TTF_Font *font = ResourceCache::GetFont(height_pixels);

  int w, h;
  TTF_SizeText(font, text.data(), &w, &h);
  wp = (float) w / winw;
}

string ReadFile(const string &filename) {
  ifstream stream(filename);
  string str;

  if (!stream) {
    stringstream ss;
    ss << "Cannot open file: " << filename;
    throw runtime_error(ss.str());
  }

  stream.seekg(0, ios::end);
  str.reserve(stream.tellg());
  stream.seekg(0, ios::beg);

  str.assign((istreambuf_iterator<char>(stream)),
             istreambuf_iterator<char>());

  return str;
}

void GetRelativeCoords(int x, int y,
                       SDL_Window *window,
                       TextAnchor xanchor, TextAnchor yanchor,
                       float &xp, float &yp)
{
  int winw, winh;
  SDL_GetWindowSize(window, &winw, &winh);

  if (xanchor == TextAnchor::LEFT)
    xp = (float) x / winw;
  else if (xanchor == TextAnchor::RIGHT)
    xp = (float) (winw - x) / winw;
  else if (xanchor == TextAnchor::CENTER)
    xp = (float) (x - winw / 2) / winw;

  if (yanchor == TextAnchor::TOP)
    yp = (float) y / winh;
  else if (yanchor == TextAnchor::BOTTOM)
    yp = (float) (winh - y) / winh;
  else if (yanchor == TextAnchor::CENTER)
    yp = (float) (winh / 2 - y) / winh;
}

void PlaySound(const string &name) {
  if (!mute) {
    int ch = Mix_PlayChannel(-1, ResourceCache::GetSound(name), 0);
    if (ch == -1)
      cout << "Warning: Error playing sound. SDL_mixer error: "
           << Mix_GetError() << endl;
  }
}
