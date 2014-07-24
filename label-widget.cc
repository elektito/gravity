#include "label-widget.hh"
#include "resource-cache.hh"
#include "screen.hh"

#include <iostream>
#include <sstream>

LabelWidget::~LabelWidget() {
  glDeleteTextures(1, &this->texture);
  glDeleteBuffers(1, &this->vbo);
}

void LabelWidget::Rebuild() {
  // Rebuild texture.
  int winw, winh;
  SDL_GetWindowSize(this->screen->window, &winw, &winh);
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
  if (this->texture != 0)
    glDeleteTextures(1, &this->texture);

  glGenTextures(1, &this->texture);
  glBindTexture(GL_TEXTURE_2D, this->texture);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, textSurface->w, textSurface->h, 0, GL_ALPHA, GL_UNSIGNED_INT, textSurface->pixels);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  // Rebuild vertex buffer object.
  float x1, y1, x2, y2, w, h;

  w = (float) textSurface->w / winw * 2.0f;
  h = (float) textSurface->h / winh * 2.0f;
  this->width = w;

  if (xanchor == TextAnchor::LEFT) {
    x1 = -1.0f + this->x * 2.0f;
  }
  else if (xanchor == TextAnchor::RIGHT) {
    x1 = 1.0f - w - this->x * 2.0f;
  }
  else if (xanchor == TextAnchor::CENTER) {
    x1 = -w / 2.0f + this->x;
  }

  if (yanchor == TextAnchor::BOTTOM) {
    y1 = -1.0f + this->y * 2.0f;
  }
  else if (yanchor == TextAnchor::TOP) {
    y1 = 1.0f - h - this->y * 2.0f;
  }
  else if (yanchor == TextAnchor::CENTER) {
    y1 = -h * 2.0f + this->y;
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

  if (this->vbo != 0)
    glDeleteBuffers(1, &this->vbo);

  glGenBuffers(1, &this->vbo);

  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
  glBufferData(GL_ARRAY_BUFFER, 6 * 4 * sizeof(GLfloat), vertexData, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Clean up.
  SDL_FreeSurface(textSurface);
}

void LabelWidget::SetText(const string &text) {
  this->text = text;
  this->Rebuild();
}

const string &LabelWidget::GetText() const {
  return this->text;
}

void LabelWidget::SetColor(const SDL_Color &c) {
  this->color = c;
}

const SDL_Color &LabelWidget::GetColor() const {
  return this->color;
}

void LabelWidget::HandleEvent(const SDL_Event &e) {
  if (e.type == SDL_WINDOWEVENT)
    if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
      this->Rebuild();
}

void LabelWidget::Advance(float dt) {

}

void LabelWidget::Render(Renderer *renderer) {
  if (!this->visible)
    return;

  GLuint program = ResourceCache::textProgram;
  glUseProgram(program);

  GLuint textureUniform = glGetUniformLocation(program, "texture0");

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, this->texture);
  glUniform1i(textureUniform, 0); // set it to 0  because the texture is bound to GL_TEXTURE0

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);

  GLuint coordAttr = glGetAttribLocation(program, "coord");
  GLuint texCoordAttr = glGetAttribLocation(program, "tex_coord");
  GLuint colorAttr = glGetAttribLocation(program, "color");

  glEnableVertexAttribArray(coordAttr);
  glEnableVertexAttribArray(texCoordAttr);

  glVertexAttribPointer(coordAttr, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*) 0);
  glVertexAttribPointer(texCoordAttr, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*) (2 * sizeof(GLfloat)));

  float r, g, b, a;
  r = (float) this->color.r / 255;
  g = (float) this->color.g / 255;
  b = (float) this->color.b / 255;
  a = (float) this->color.a / 255;
  glVertexAttrib3f(colorAttr, r, g, b);

  glDrawArrays(GL_TRIANGLES, 0, 6);
  if (glGetError() != GL_NO_ERROR)
    cout << "OpenGL draw error." << endl;

  glDisableVertexAttribArray(coordAttr);
  glDisableVertexAttribArray(texCoordAttr);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glUseProgram(0);
}

void LabelWidget::Reset() {
  this->Rebuild();
}
