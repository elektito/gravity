#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_image.h>

#include <tuple>
#include <string>
#include <sstream>
#include <map>

using namespace std;

namespace ResourceCache {

SDL_Renderer *renderer = nullptr;

struct FontDescriptor {
  string path;
  int size;

  bool operator<(const FontDescriptor &rhs) const {
    return std::tie(this->path, this->size) < std::tie(rhs.path, rhs.size);
  }
};

map<FontDescriptor, TTF_Font*> font_cache;
map<string, Mix_Chunk*> sound_cache;
map<string, SDL_Texture*> image_cache;

void Init(SDL_Renderer *r) {
  stringstream ss;

  renderer = r;

  // Initialize fonts.
  if (TTF_Init() == -1) {
    ss << "Could not initialize SDL_ttf. SDL_ttf error: "
       << TTF_GetError();
    throw runtime_error(ss.str());
  }

  // Initialize sounds.
  if (Mix_Init(0) == -1) {
    ss << "Could not initialize SDL_mixer. SDL_mixer error: "
       << Mix_GetError();
    throw runtime_error(ss.str());
  }

  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
    ss << "Could not initialize SDL_mixer. SDL_mixer error: "
       << Mix_GetError();
    throw runtime_error(ss.str());
  }

  // Initialize images.
  if (IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) == -1) {
    ss << "Could not initialize SDL_image. SDL_image error: "
       << IMG_GetError();
    throw runtime_error(ss.str());
  }
}

void Finalize() {
  for (auto p : font_cache)
    TTF_CloseFont(p.second);

  for (auto p : sound_cache)
    Mix_FreeChunk(p.second);

  TTF_Quit();
  Mix_Quit();
  IMG_Quit();
}

TTF_Font *GetFont(int height_pixels) {
  FontDescriptor desc {"fonts/UbuntuMono-B.ttf", height_pixels};

  auto it = font_cache.find(desc);
  if (it != font_cache.end())
    return it->second;

  TTF_Font *font = TTF_OpenFont("resources/fonts/UbuntuMono-B.ttf", height_pixels);
  if (font == nullptr) {
    stringstream ss;
    ss << "Unable to load font. SDL_ttf error: " << TTF_GetError();
    throw runtime_error(ss.str());
  }

  font_cache[desc] = font;

  return font;
}

Mix_Chunk *GetSound(const string &name) {
  auto it = sound_cache.find(name);
  if (it != sound_cache.end())
    return it->second;

  Mix_Chunk *chunk = Mix_LoadWAV(("resources/sound/" + name + ".wav").data());
  if (chunk == nullptr) {
    stringstream ss;
    ss << "Unable to load sound. SDL_mixer error: " << Mix_GetError();
    throw runtime_error(ss.str());
  }

  sound_cache[name] = chunk;

  return chunk;
}

SDL_Texture *GetImage(const string &name, const string &type) {
  auto it = image_cache.find(name);
  if (it != image_cache.end())
    return it->second;

  SDL_Surface *surface = IMG_Load(("resources/images/" + name + "." + type).data());
  if (surface == nullptr) {
    stringstream ss;
    ss << "Unable to load image. SDL_image error: " << IMG_GetError();
    throw runtime_error(ss.str());
  }

  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
  if (texture == nullptr) {
    stringstream ss;
    ss << "Unable to create texture. SDL error: " << SDL_GetError();
    throw runtime_error(ss.str());
  }

  image_cache[name] = texture;

  return texture;
}

} // namespace ResourceCache
