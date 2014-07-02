#ifndef _GRAVITY_RESOURCE_CACHE_HH_
#define _GRAVITY_RESOURCE_CACHE_HH_

#include <SDL2/SDL_ttf.h>

namespace ResourceCache {

extern void Init();
extern void Finalize();
extern TTF_Font *GetFont(int height_pixels);

} // namespace ResourceCache

#endif /* _GRAVITY_RESOURCE_CACHE_HH_ */
