#include "renderer.hh"
#include "resource-cache.hh"
#include "platform.hh"

#include <iostream>
#include <sstream>

using namespace std;

Background::Background(SDL_Window *window, ResourceCache::Texture texture) :
  window(window),
  texture(texture),
  lastWindowWidth(0),
  lastWindowHeight(0)
{
  // Create background vertex buffer object.
  this->RebuildIfNecessary();
}

Background::~Background() {
  glDeleteBuffers(1, &this->vbo);
  glDeleteTextures(1, &this->texture.id);
}

void Background::RebuildIfNecessary() {
  int winw, winh;
  SDL_GetWindowSize(this->window, &winw, &winh);
  if (winw == this->lastWindowWidth && winh == this->lastWindowHeight)
    return;

  this->lastWindowWidth = winw;
  this->lastWindowHeight = winh;

  float windowRatio = (float) winw / winh;

  GLfloat tex_x1, tex_y1, tex_x2, tex_y2;
  float textureRatio = (float) this->texture.width / this->texture.height;
  if (textureRatio < windowRatio) {
    tex_x1 = 0.0f;
    tex_x2 = 1.0f;
    tex_y1 = 0.5f * ((float) winw / this->texture.width - (float) winh / this->texture.height);
    tex_y2 = 1.0 - 0.5f * ((float) winw / this->texture.width - (float) winh / this->texture.height);
  }
  else {
    tex_y1 = 0.0f;
    tex_y2 = 1.0f;
    tex_x1 = 0.5f * ((float) winh / this->texture.height - (float) winw / this->texture.width);
    tex_x2 = 1.0 - 0.5f * ((float) winh / this->texture.height - (float) winw / this->texture.width);
  }

  const GLfloat vertexData[] = {
    // triangle 1
    /* coord */ -1.0f, -1.0f, /* tex_coord */ tex_x1, tex_y1,
    /* coord */ -1.0f,  1.0f, /* tex_coord */ tex_x1, tex_y2,
    /* coord */  1.0f, -1.0f, /* tex_coord */ tex_x2, tex_y1,

    // triangle 2
    /* coord */ -1.0f,  1.0f, /* tex_coord */ tex_x1, tex_y2,
    /* coord */  1.0f,  1.0f, /* tex_coord */ tex_x2, tex_y2,
    /* coord */  1.0f, -1.0f, /* tex_coord */ tex_x2, tex_y1,
  };

  glGenBuffers(1, &this->vbo);

  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
  glBufferData(GL_ARRAY_BUFFER, 6 * 4 * sizeof(GLfloat), vertexData, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Background::Draw() {
  this->RebuildIfNecessary();

  GLuint program = ResourceCache::backgroundProgram;

  glUseProgram(program);

  GLuint textureUniform = glGetUniformLocation(program, "texture0");

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, this->texture.id);
  glUniform1i(textureUniform, 0); // set it to 0  because the texture is bound to GL_TEXTURE0

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);

  GLint coordAttr = glGetAttribLocation(program, "coord");
  GLint texCoordAttr = glGetAttribLocation(program, "tex_coord");

  glEnableVertexAttribArray(coordAttr);
  glEnableVertexAttribArray(texCoordAttr);

  glVertexAttribPointer(coordAttr, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*) 0);
  glVertexAttribPointer(texCoordAttr, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*) (2 * sizeof(GLfloat)));

  glDrawArrays(GL_TRIANGLES, 0, 6);
  if (glGetError() != GL_NO_ERROR)
    cout << "renderer: OpenGL draw error." << endl;

  glDisableVertexAttribArray(coordAttr);
  glDisableVertexAttribArray(texCoordAttr);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glUseProgram(0);
}

Renderer::Renderer(SDL_Window *window) :
  window(window)
{
  // Create an OpenGL context for the window and make it current. This
  // is needed for the subsequent call to glewInit.
  this->context = SDL_GL_CreateContext(window);

  // initialize glew
  GLenum status = glewInit();
  if (status != GLEW_OK) {
    SHOW_MSG("Could not initialize GLEW.");
    exit(1);
  }

  if (!GLEW_VERSION_3_3) {
    SHOW_MSG("OpenGL version 3.3 not found. Make sure your video card driver is up to date.");
    exit(1);
  }

  // Enable blending.
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

Renderer::~Renderer() {

}

void Renderer::PresentScreen() const {
  SDL_GL_SwapWindow(this->window);
}

void Renderer::SetCamera(Camera &camera) {
  this->camera = camera;
}

void Renderer::ClearScreen() {
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
}
