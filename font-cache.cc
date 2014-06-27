#include <SDL2/SDL_ttf.h>

#include <tuple>
#include <string>
#include <sstream>
#include <map>

using namespace std;

namespace FontCache {

struct FontDescriptor {
  string path;
  int size;

  bool operator<(const FontDescriptor &rhs) const {
    return std::tie(this->path, this->size) < std::tie(rhs.path, rhs.size);
  }
};

map <FontDescriptor, TTF_Font*> font_cache;

void Init() {
  // Initialize fonts.
  if (TTF_Init() == -1) {
    stringstream ss;
    ss << "Could not initialize SDL_ttf. SDL_ttf error: "
       << TTF_GetError();
    throw runtime_error(ss.str());
  }
}

void Finalize() {
  for (auto p : font_cache)
    TTF_CloseFont(p.second);

  TTF_Quit();
}

TTF_Font *GetFont(int height_pixels) {
  FontDescriptor desc {"fonts/UbuntuMono-B.ttf", height_pixels};

  auto it = font_cache.find(desc);
  if (it != font_cache.end())
    return it->second;

  TTF_Font *font = TTF_OpenFont("fonts/UbuntuMono-B.ttf", height_pixels);
  if (font == nullptr) {
    stringstream ss;
    ss << "Unable to load font. SDL_ttf error: " << TTF_GetError();
    throw runtime_error(ss.str());
  }

  font_cache[desc] = font;

  return font;
}

} // namespace FontCache
