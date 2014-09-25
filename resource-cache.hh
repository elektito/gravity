#ifndef _GRAVITY_RESOURCE_CACHE_HH_
#define _GRAVITY_RESOURCE_CACHE_HH_

#include "glew.h"

#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include <string>

using namespace std;

namespace ResourceCache {

extern string RESOURCES_PATH;

extern GLuint texturedPolygonProgram;
extern GLuint hudTexturedPolygonProgram;
extern GLuint textProgram;
extern GLuint backgroundProgram;

extern void Init();
extern void Finalize();

extern TTF_Font *GetFont(int height_pixels);
extern Mix_Chunk *GetSound(const string &name);
extern GLuint GetTexture(const string &name, const string &type="png");

} // namespace ResourceCache

#endif /* _GRAVITY_RESOURCE_CACHE_HH_ */
