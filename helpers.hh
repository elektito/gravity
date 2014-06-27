#ifndef _GRAVITY_STREAMS_HH_
#define _GRAVITY_STREAMS_HH_

#include <Box2D/Box2D.h>
#include <SDL2/SDL.h>

#include <iostream>
#include <map>

using namespace std;

extern ostream &operator<<(ostream &s, const b2Vec2 &v);
extern istream &operator>>(istream &s, b2Vec2 &v);

extern void SaveMap(const map<string, string> &m, ostream &s);
extern void LoadMap(map<string, string> &m, istream &s);

extern void GetTextWidthP(string text, float32 hp, SDL_Window *window, float32 &wp);

#endif /* _GRAVITY_STREAMS_HH_ */
