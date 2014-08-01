#ifndef _GRAVITY_STREAMS_HH_
#define _GRAVITY_STREAMS_HH_

#include "renderer.hh"

#include <Box2D/Box2D.h>
#include <SDL2/SDL.h>

#include <iostream>
#include <map>

using namespace std;

#define WRITE(DATA, STREAM) (STREAM).write((const char*) &DATA, sizeof(DATA))
#define READ(DATA, STREAM) (STREAM).read((char*) &DATA, sizeof(DATA));

extern bool mute;

extern void SaveVec2(const b2Vec2 &v, ostream &s);
extern void LoadVec2(b2Vec2 &v, istream &s);
extern void SaveString(const string &str, ostream &s);
extern void LoadString(string &str, istream &s);
extern void SaveMap(const map<string, string> &m, ostream &s);
extern void LoadMap(map<string, string> &m, istream &s);

extern void GetTextWidthP(string text, float32 hp, SDL_Window *window, float32 &wp);

extern string ReadFile(const string &filename);

/// Converts the window coordinate (x, y) to relative coordinates (xp,
/// yp), i.e. xp and yp in range [0.0, 1.0], for a widget with the
/// given width and height in the given window.
extern void GetRelativeCoords(int x, int y,
                              SDL_Window *window,
                              TextAnchor xanchor, TextAnchor yanchor,
                              float &xp, float &yp);

extern void PlaySound(const string &name);

#endif /* _GRAVITY_STREAMS_HH_ */
