#ifndef _GRAVITY_STREAMS_HH_
#define _GRAVITY_STREAMS_HH_

#include <Box2D/Box2D.h>
#include <SDL2/SDL.h>

#include <iostream>
#include <map>

using namespace std;

#define WRITE(DATA, STREAM) (STREAM).write((const char*) &DATA, sizeof(DATA))
#define READ(DATA, STREAM) (STREAM).read((char*) &DATA, sizeof(DATA));

extern void SaveVec2(const b2Vec2 &v, ostream &s);
extern void LoadVec2(b2Vec2 &v, istream &s);
extern void SaveString(const string &str, ostream &s);
extern void LoadString(string &str, istream &s);
extern void SaveMap(const map<string, string> &m, ostream &s);
extern void LoadMap(map<string, string> &m, istream &s);

extern void GetTextWidthP(string text, float32 hp, SDL_Window *window, float32 &wp);

extern string ReadFile(const string &filename);

#endif /* _GRAVITY_STREAMS_HH_ */
