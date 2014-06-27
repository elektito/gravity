#ifndef _GRAVITY_FONTCACHE_HH_
#define _GRAVITY_FONTCACHE_HH_

#include <SDL2/SDL_ttf.h>

#include <string>

using namespace std;

namespace FontCache {

extern void Init();
extern void Finalize();
extern TTF_Font *GetFont(int height_pixels);

} // namespace FontCache

#endif /* _GRAVITY_FONTCACHE_HH_ */
