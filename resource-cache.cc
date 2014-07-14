#include "resource-cache.hh"
#include "helpers.hh"

#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <GL/glew.h>
#include <SOIL/SOIL.h>

#include <tuple>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <algorithm>

using namespace std;

namespace ResourceCache {

GLuint texturedPolygonProgram = 0;
GLuint textProgram = 0;
GLuint backgroundProgram = 0;

struct FontDescriptor {
  string path;
  int size;

  bool operator<(const FontDescriptor &rhs) const {
    return std::tie(this->path, this->size) < std::tie(rhs.path, rhs.size);
  }
};

map<GLenum, string> shaderTypeNames;
map<FontDescriptor, TTF_Font*> font_cache;
map<string, Mix_Chunk*> sound_cache;
map<string, GLuint> texture_cache;

GLuint CreateShader(GLenum shaderType, const string &shaderSource) {
  string shaderTypeName = shaderTypeNames[shaderType];

  GLuint shader = glCreateShader(shaderType);
  const char *shaderSourceData = shaderSource.data();
  glShaderSource(shader, 1, &shaderSourceData, NULL);

  glCompileShader(shader);

  // Get compilation log.
  GLint infoLogLength;
  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

  GLchar *infoLog = new GLchar[infoLogLength + 1];
  glGetShaderInfoLog(shader, infoLogLength, NULL, infoLog);

  if (infoLogLength > 1) {
    cout << "Compilation log for " << shaderTypeName << " shader:" << endl << endl;
    cout << infoLog << endl;
  }

  delete[] infoLog;

  GLint status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE) {
    cout << "Compile failure in " << shaderTypeName << " shader." << endl;
    exit(1);
  }

  return shader;
}

GLuint CreateProgram(const vector<GLuint> &shaders) {
  GLuint program = glCreateProgram();

  for (auto shader : shaders)
    glAttachShader(program, shader);

  glLinkProgram(program);

  GLint status;
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  if (status == GL_FALSE) {
    GLint infoLogLength;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

    GLchar *infoLog = new GLchar[infoLogLength];
    glGetProgramInfoLog(program, infoLogLength, NULL, infoLog);

    cout << "Linker failure: " << infoLog << endl;
    delete[] infoLog;
    exit(1);
  }

  return program;
}

void Init() {
  stringstream ss;

  // Initialize fonts.
  cout << "Initializing font system..." << endl;
  if (TTF_Init() == -1) {
    ss << "Could not initialize SDL_ttf. SDL_ttf error: "
       << TTF_GetError();
    throw runtime_error(ss.str());
  }

  // Initialize sounds.
  cout << "Initializing sound system..." << endl;
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

  // Compile shaders.
  cout << "Compiling shaders..." << endl;

  shaderTypeNames[GL_VERTEX_SHADER] = "vertex";
  shaderTypeNames[GL_GEOMETRY_SHADER] = "geometry";
  shaderTypeNames[GL_FRAGMENT_SHADER] = "fragment";

  vector<GLuint> shaders;
  string vertexShaderSource = ReadFile("resources/shaders/tex-poly-vertex-shader.glsl");
  string fragmentShaderSource = ReadFile("resources/shaders/tex-poly-fragment-shader.glsl");
  shaders.push_back(CreateShader(GL_VERTEX_SHADER, vertexShaderSource));
  shaders.push_back(CreateShader(GL_FRAGMENT_SHADER, fragmentShaderSource));

  texturedPolygonProgram = CreateProgram(shaders);

  for_each(shaders.begin(), shaders.end(), glDeleteShader);
  shaders.clear();

  vertexShaderSource = ReadFile("resources/shaders/text-vertex-shader.glsl");
  fragmentShaderSource = ReadFile("resources/shaders/text-fragment-shader.glsl");
  shaders.push_back(CreateShader(GL_VERTEX_SHADER, vertexShaderSource));
  shaders.push_back(CreateShader(GL_FRAGMENT_SHADER, fragmentShaderSource));

  textProgram = CreateProgram(shaders);

  for_each(shaders.begin(), shaders.end(), glDeleteShader);
  shaders.clear();

  vertexShaderSource = ReadFile("resources/shaders/background-vertex-shader.glsl");
  fragmentShaderSource = ReadFile("resources/shaders/background-fragment-shader.glsl");
  shaders.push_back(CreateShader(GL_VERTEX_SHADER, vertexShaderSource));
  shaders.push_back(CreateShader(GL_FRAGMENT_SHADER, fragmentShaderSource));

  backgroundProgram = CreateProgram(shaders);

  for_each(shaders.begin(), shaders.end(), glDeleteShader);
  shaders.clear();

  // Preload textures.
  cout << "Pre-loading textures..." << endl;
  GetTexture("sun");
  GetTexture("planet");
  GetTexture("plus-score");
  GetTexture("enemy");

  cout << "Resource cache initialized." << endl;
}

void Finalize() {
  for (auto p : font_cache)
    TTF_CloseFont(p.second);

  for (auto p : sound_cache)
    Mix_FreeChunk(p.second);

  TTF_Quit();
  Mix_Quit();
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

GLuint GetTexture(const string &name, const string &type) {
  auto it = texture_cache.find(name);
  if (it != texture_cache.end())
    return it->second;

  GLuint texture = SOIL_load_OGL_texture(("resources/images/" + name + "." + type).data(),
                                         SOIL_LOAD_AUTO,
                                         SOIL_CREATE_NEW_ID,
                                         SOIL_FLAG_MIPMAPS |
                                         SOIL_FLAG_INVERT_Y |
                                         SOIL_FLAG_NTSC_SAFE_RGB |
                                         SOIL_FLAG_COMPRESS_TO_DXT);
  if (texture == 0) {
    stringstream ss;
    ss << "Unable to load image. SOIL error: "
       << SOIL_last_result();
    throw runtime_error(ss.str());
  }

  texture_cache[name] = texture;

  return texture;
}

} // namespace ResourceCache
