#include "resource-cache.hh"
#include "helpers.hh"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <GL/glew.h>

#include <tuple>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <algorithm>

using namespace std;

namespace ResourceCache {

GLuint texturedPolygonProgram = 0;
GLuint hudTexturedPolygonProgram = 0;
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

GLuint CreateProgram(string vertexShaderFilename, string fragmentShaderFilename) {
  vector<GLuint> shaders;
  string vertexShaderSource = ReadFile(vertexShaderFilename);
  string fragmentShaderSource = ReadFile(fragmentShaderFilename);
  shaders.push_back(CreateShader(GL_VERTEX_SHADER, vertexShaderSource));
  shaders.push_back(CreateShader(GL_FRAGMENT_SHADER, fragmentShaderSource));

  GLuint program = CreateProgram(shaders);
  for_each(shaders.begin(), shaders.end(), glDeleteShader);

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

  texturedPolygonProgram = CreateProgram("resources/shaders/tex-poly-vertex-shader.glsl",
                                         "resources/shaders/tex-poly-fragment-shader.glsl");

  hudTexturedPolygonProgram = CreateProgram("resources/shaders/hud-tex-poly-vertex-shader.glsl",
                                            "resources/shaders/tex-poly-fragment-shader.glsl");

  textProgram = CreateProgram("resources/shaders/text-vertex-shader.glsl",
                              "resources/shaders/text-fragment-shader.glsl");

  backgroundProgram = CreateProgram("resources/shaders/background-vertex-shader.glsl",
                                    "resources/shaders/background-fragment-shader.glsl");

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

  TTF_Font *font = TTF_OpenFont("resources/fonts/kenvector_future.ttf", height_pixels);
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

int
        downscale_image
        (
                const unsigned char* const orig,
                int width, int height, int channels,
                unsigned char* resampled,
                int block_size_x, int block_size_y,
                int new_width, int new_height
        )
{
        int mip_width, mip_height;
        int i, j, c;

        /*	error check	*/
        if( (width < 1) || (height < 1) ||
                (channels < 1) || (orig == NULL) ||
                (resampled == NULL) ||
                (block_size_x < 1) || (block_size_y < 1) )
        {
                /*	nothing to do	*/
                return 0;
        }
        mip_width = new_width;// width / block_size_x;
        mip_height = new_height;// height / block_size_y;
        /*if( mip_width < 1 )
        {
                mip_width = 1;
        }
        if( mip_height < 1 )
        {
                mip_height = 1;
                }*/
        for( j = 0; j < mip_height; ++j )
        {
                for( i = 0; i < mip_width; ++i )
                {
                        for( c = 0; c < channels; ++c )
                        {
                                const int index = (j*block_size_y)*width*channels + (i*block_size_x)*channels + c;
                                int sum_value;
                                int u,v;
                                int u_block = block_size_x;
                                int v_block = block_size_y;
                                int block_area;
                                /*	do a bit of checking so we don't over-run the boundaries
                                        (necessary for non-square textures!)	*/
                                if( block_size_x * (i+1) > width )
                                {
                                        u_block = width - i*block_size_y;
                                }
                                if( block_size_y * (j+1) > height )
                                {
                                        v_block = height - j*block_size_y;
                                }
                                block_area = u_block*v_block;
                                /*	for this pixel, see what the average
                                        of all the values in the block are.
                                        note: start the sum at the rounding value, not at 0	*/
                                sum_value = block_area >> 1;
                                for( v = 0; v < v_block; ++v )
                                for( u = 0; u < u_block; ++u )
                                {
                                        sum_value += orig[index + v*width*channels + u*channels];
                                }
                                resampled[j*mip_width*channels + i*channels + c] = sum_value / block_area;
                        }
                }
        }
        return 1;
}

GLuint GetTexture(const string &name, const string &type) {
  auto it = texture_cache.find(name);
  if (it != texture_cache.end())
    return it->second;

  int w, h, channels;
  uint8_t *img = stbi_load(("resources/images/" + name + "." + type).data(), &w, &h, &channels, 0);
  if (img == nullptr) {
    stringstream ss;
    ss << "Unable to load image. stb_image error: "
       << stbi_failure_reason();
    throw runtime_error(ss.str());
  }
  cout << w << " " << h << " " << channels << " " << GL_MAX_TEXTURE_SIZE << endl;

  int nw = w;
  int nh = h;
  while (nw > GL_MAX_TEXTURE_SIZE || h > GL_MAX_TEXTURE_SIZE) {
    nw /= 2;
    nh /= 2;
    cout << "resize: nw=" << nw << " nh=" << nh << endl;
  }

  if (nw != w || nh != h) {
    cout << "[" << name << "] obtaining: " << nw * nh * channels << " (nw=" << nw << ", nh=" << nh << ", ch=" << channels << ")" << endl;
    uint8_t *resampled = new uint8_t[nw * nh * channels];
    int bx = w / nw;
    int by = h / nh;
    cout << "bx=" << bx << " by=" << by << endl;
    downscale_image(img, w, h, channels, resampled, bx, by, nw, nh);
    stbi_image_free(img);
    img = resampled;
    w = nw;
    h = nh;
  }

  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
  auto err = glGetError();
  if (err != GL_NO_ERROR)
    cout << "OpenGL error " << err << " while loading image: " << name << endl;
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glBindTexture(GL_TEXTURE_2D, 0);

  // Generate mipmaps.
  glBindTexture(GL_TEXTURE_2D, texture);
  glGenerateMipmap(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);

  texture_cache[name] = texture;

  return texture;
}

} // namespace ResourceCache
