#include "renderer.hh"
#include "resource-cache.hh"

#include <iostream>
#include <sstream>

using namespace std;

Renderer::Renderer(SDL_Window *window) :
  window(window)
{
  this->context = SDL_GL_CreateContext(window);

  // initialize glew
  GLenum status = glewInit();
  if (status != GLEW_OK) {
    cout << "Could not initialize GLEW." << endl;
    exit(1);
  }

  if (!GLEW_VERSION_3_3) {
    cout << "GLEW version 3.3 not found." << endl;
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

void Renderer::DrawText(const string &text,
                        float x, float y, float height,
                        const SDL_Color &color,
                        TextAnchor xanchor, TextAnchor yanchor)
{
  int winw, winh;
  SDL_GetWindowSize(this->window, &winw, &winh);
  int height_pixels = height * winh;

  TTF_Font *font = ResourceCache::GetFont(height_pixels);
  SDL_Surface *textSurface = TTF_RenderText_Blended(font, text.data(), color);
  if (textSurface == nullptr) {
    stringstream ss;
    ss << "Unable to render text surface. SDL_ttf error: "
       << TTF_GetError();
    throw runtime_error(ss.str());
  }

  // Convert text to an OpenGL texture.
  GLuint texture;

  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, textSurface->w, textSurface->h, 0, GL_ALPHA, GL_UNSIGNED_INT, textSurface->pixels);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Calculate vertices.
  float x1, y1, x2, y2, w, h;

  w = (float) textSurface->w / winw * 2.0f;
  h = (float) textSurface->h / winh * 2.0f;

  if (xanchor == TextAnchor::LEFT) {
    x1 = -1.0f + x * 2.0f;
  }
  else if (xanchor == TextAnchor::RIGHT) {
    x1 = 1.0f - w - x * 2.0f;
  }
  else if (xanchor == TextAnchor::CENTER) {
    x1 = -w / 2.0f + x;
  }

  if (yanchor == TextAnchor::BOTTOM) {
    y1 = -1.0f + y * 2.0f;
  }
  else if (yanchor == TextAnchor::TOP) {
    y1 = 1.0f - h - y * 2.0f;
  }
  else if (yanchor == TextAnchor::CENTER) {
    y1 = -h * 2.0f + y;
  }

  x2 = x1 + w;
  y2 = y1 + h;

  const GLfloat vertexData[] = {
    // triangle 1
    /* coord */ x1, y1, /* tex_coord */ 0.0f, 1.0f,
    /* coord */ x1, y2, /* tex_coord */ 0.0f, 0.0f,
    /* coord */ x2, y1, /* tex_coord */ 1.0f, 1.0f,

    // triangle 2
    /* coord */ x1, y2, /* tex_coord */ 0.0f, 0.0f,
    /* coord */ x2, y2, /* tex_coord */ 1.0f, 0.0f,
    /* coord */ x2, y1, /* tex_coord */ 1.0f, 1.0f,
  };

  GLuint vbo;
  glGenBuffers(1, &vbo);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, 6 * 4 * sizeof(GLfloat), vertexData, GL_STREAM_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Render.
  GLuint program = ResourceCache::textProgram;
  glUseProgram(program);

  GLuint textureUniform = glGetUniformLocation(program, "texture0");

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);
  glUniform1i(textureUniform, 0); // set it to 0  because the texture is bound to GL_TEXTURE0

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  GLuint coordAttr = glGetAttribLocation(program, "coord");
  GLuint texCoordAttr = glGetAttribLocation(program, "tex_coord");
  GLuint colorAttr = glGetAttribLocation(program, "color");

  glEnableVertexAttribArray(coordAttr);
  glEnableVertexAttribArray(texCoordAttr);

  glVertexAttribPointer(coordAttr, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*) 0);
  glVertexAttribPointer(texCoordAttr, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*) (2 * sizeof(GLfloat)));

  float r, g, b;
  r = (float) color.r / 255;
  g = (float) color.g / 255;
  b = (float) color.b / 255;
  glVertexAttrib3f(colorAttr, r, g, b);

  glDrawArrays(GL_TRIANGLES, 0, 6);
  if (glGetError() != GL_NO_ERROR)
    cout << "OpenGL draw error." << endl;

  glDisableVertexAttribArray(coordAttr);
  glDisableVertexAttribArray(texCoordAttr);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glUseProgram(0);

  // Clean up.
  SDL_FreeSurface(textSurface);
  glDeleteBuffers(1, &vbo);
}
