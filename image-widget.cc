#include "image-widget.hh"
#include "resource-cache.hh"

#include <iostream>

ImageWidget::ImageWidget(Screen *screen, GLuint texture, float x, float y, float height, TextAnchor xanchor, TextAnchor yanchor, const SDL_Color &color) :
    Widget(screen),
    x(x),
    y(y),
    height(height),
    xanchor(xanchor),
    yanchor(yanchor),
    texture(texture),
    color({color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f})
{
  const GLfloat vertexData[] = {
    // triangle 1
    /* coord */ 0.0f, 0.0f, /* tex_coord */ 0.0f, 0.0f,
    /* coord */ 0.0f, 1.0f, /* tex_coord */ 0.0f, 1.0f,
    /* coord */ 1.0f, 0.0f, /* tex_coord */ 1.0f, 0.0f,

    // triangle 2
    /* coord */ 0.0f, 1.0f, /* tex_coord */ 0.0f, 1.0f,
    /* coord */ 1.0f, 1.0f, /* tex_coord */ 1.0f, 1.0f,
    /* coord */ 1.0f, 0.0f, /* tex_coord */ 1.0f, 0.0f,
  };

  glGenBuffers(1, &this->vbo);

  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
  glBufferData(GL_ARRAY_BUFFER, 6 * 4 * sizeof(GLfloat), vertexData, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  int textureWidth, textureHeight;
  glBindTexture(GL_TEXTURE_2D, this->texture);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &textureWidth);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &textureHeight);
  glBindTexture(GL_TEXTURE_2D, 0);

  float ratio = (float) textureWidth / textureHeight;
  this->width = height * ratio;
}

ImageWidget::~ImageWidget() {
  glDeleteBuffers(1, &this->vbo);
}

void ImageWidget::SetColor(float r, float g, float b, float a) {
  this->color.r = r;
  this->color.g = g;
  this->color.b = b;
  this->color.a = a;
}

void ImageWidget::HandleEvent(const SDL_Event &e) {

}

void ImageWidget::Advance(float dt) {

}

void ImageWidget::Render(Renderer *renderer) {
  if (!this->visible)
    return;

  GLuint program = ResourceCache::hudTexturedPolygonProgram;

  glUseProgram(program);

  GLuint textureUniform = glGetUniformLocation(program, "texture0");

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, this->texture);
  glUniform1i(textureUniform, 0); // set it to 0  because the texture is bound to GL_TEXTURE0

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);

  GLint coordAttr = glGetAttribLocation(program, "coord");
  GLint texCoordAttr = glGetAttribLocation(program, "tex_coord");
  GLint positionAttr = glGetAttribLocation(program, "position");
  GLint xalignAttr = glGetAttribLocation(program, "xalign");
  GLint yalignAttr = glGetAttribLocation(program, "yalign");
  GLint widthAttr = glGetAttribLocation(program, "width");
  GLint heightAttr = glGetAttribLocation(program, "height");
  GLint colorAttr = glGetAttribLocation(program, "color");

  glEnableVertexAttribArray(coordAttr);
  glEnableVertexAttribArray(texCoordAttr);

  glVertexAttribPointer(coordAttr, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*) 0);
  glVertexAttribPointer(texCoordAttr, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*) (2 * sizeof(GLfloat)));

  int shaderXAnchor, shaderYAnchor;
  if (this->xanchor == TextAnchor::LEFT)
    shaderXAnchor = 1;
  else if (this->xanchor == TextAnchor::CENTER)
    shaderXAnchor = 2;
  else
    shaderXAnchor = 3;
  if (this->yanchor == TextAnchor::BOTTOM)
    shaderYAnchor = 1;
  else if (this->yanchor == TextAnchor::CENTER)
    shaderYAnchor = 2;
  else
    shaderYAnchor = 3;
  glVertexAttrib2f(positionAttr, this->x, this->y);
  glVertexAttribI1i(xalignAttr, shaderXAnchor);
  glVertexAttribI1i(yalignAttr, shaderYAnchor);
  glVertexAttrib1f(widthAttr, this->width);
  glVertexAttrib1f(heightAttr, this->height);
  glVertexAttrib4f(colorAttr, this->color.r, this->color.g, this->color.b, this->color.a);

  glDrawArrays(GL_TRIANGLES, 0, 6);
  if (glGetError() != GL_NO_ERROR)
    cout << "OpenGL draw error." << endl;

  glDisableVertexAttribArray(coordAttr);
  glDisableVertexAttribArray(texCoordAttr);

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glUseProgram(0);
}

void ImageWidget::Reset() {

}
