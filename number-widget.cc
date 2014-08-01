#include "number-widget.hh"
#include "resource-cache.hh"

#include <iostream>
#include <sstream>
#include <iomanip>

using namespace std;

NumberWidget::NumberWidget(Screen *screen, uint32_t n, float x, float y, float height, TextAnchor xanchor, TextAnchor yanchor, uint32_t ndigits, const SDL_Color &color) :
  Widget(screen),
  x(x),
  y(y),
  height(height),
  xanchor(xanchor),
  yanchor(yanchor),
  ndigits(ndigits),
  color({color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f}),
  vbo(0)
{
  if (ndigits = 0)
    throw runtime_error("Zero digits not acceptable for number widget.");

  this->SetNumber(n);
}

NumberWidget::~NumberWidget() {
  if (this->vbo)
    glDeleteBuffers(1, &this->vbo);
}

void NumberWidget::SetNumber(uint32_t n) {
  if (this->vbo)
    glDeleteBuffers(1, &this->vbo);

  stringstream ss;
  ss << setw(this->ndigits) << setfill('0') << n;
  string str = ss.str();

  if (str.size() != this->ndigits)
    throw runtime_error("Invalid number for number widget.");

  GLfloat *vertexData = new GLfloat[4 * 6 * this->ndigits];
  float step = 1.0f / this->ndigits;
  float dstep = 0.1;

  for (int i = 0; i < this->ndigits; ++i) {
    int d = std::stoi(str.substr(i, 1));

    // triangle 1, vertex 1
    vertexData[i * 6 * 4 + 0] = i * step; // coord.x
    vertexData[i * 6 * 4 + 1] = 0.0f;     // coord.y
    vertexData[i * 6 * 4 + 2] = d * dstep; // tex_coord.x
    vertexData[i * 6 * 4 + 3] = 0.0f;     // tex_coord.y;

    // triangle 1, vertex 2
    vertexData[i * 6 * 4 + 4] = i * step; // coord.x
    vertexData[i * 6 * 4 + 5] = 1.0f;     // coord.y
    vertexData[i * 6 * 4 + 6] = d * dstep; // tex_coord.x
    vertexData[i * 6 * 4 + 7] = 1.0f;     // tex_coord.y;

    // triangle 1, vertex 3
    vertexData[i * 6 * 4 + 8] = (i + 1) * step;  // coord.x
    vertexData[i * 6 * 4 + 9] = 1.0f;            // coord.y
    vertexData[i * 6 * 4 + 10] = (d + 1) * dstep; // tex_coord.x
    vertexData[i * 6 * 4 + 11] = 1.0f;           // tex_coord.y;

    // triangle 2, vertex 1
    vertexData[i * 6 * 4 + 12] = (i + 1) * step; // coord.x
    vertexData[i * 6 * 4 + 13] = 1.0f;           // coord.y
    vertexData[i * 6 * 4 + 14] = (d + 1) * dstep; // tex_coord.x
    vertexData[i * 6 * 4 + 15] = 1.0f;           // tex_coord.y;

    // triangle 2, vertex 2
    vertexData[i * 6 * 4 + 16] = (i + 1) * step; // coord.x
    vertexData[i * 6 * 4 + 17] = 0.0f;           // coord.y
    vertexData[i * 6 * 4 + 18] = (d + 1) * dstep; // tex_coord.x
    vertexData[i * 6 * 4 + 19] = 0.0f;           // tex_coord.y;

    // triangle 2, vertex 3
    vertexData[i * 6 * 4 + 20] = i * step; // coord.x
    vertexData[i * 6 * 4 + 21] = 0.0f;     // coord.y
    vertexData[i * 6 * 4 + 22] = d * dstep; // tex_coord.x
    vertexData[i * 6 * 4 + 23] = 0.0f;     // tex_coord.y;
  }

  glGenBuffers(1, &this->vbo);

  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
  glBufferData(GL_ARRAY_BUFFER, this->ndigits * 6 * 4 * sizeof(GLfloat), vertexData, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  int textureWidth, textureHeight;
  glBindTexture(GL_TEXTURE_2D, ResourceCache::GetTexture("digits"));
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &textureWidth);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &textureHeight);
  glBindTexture(GL_TEXTURE_2D, 0);

  float ratio = (float) (textureWidth / 10.0f * this->ndigits) / textureHeight;
  this->width = height * ratio;

  delete vertexData;
}

void NumberWidget::SetColor(float r, float g, float b, float a) {
  this->color.r = r;
  this->color.g = g;
  this->color.b = b;
  this->color.a = a;
}

void NumberWidget::SetColor(const SDL_Color &c) {
  this->color.r = c.r / 255.0;
  this->color.g = c.g / 255.0;
  this->color.b = c.b / 255.0;
  this->color.a = c.a / 255.0;
}

void NumberWidget::HandleEvent(const SDL_Event &e) {

}

void NumberWidget::Advance(float dt) {

}

void NumberWidget::Render(Renderer *renderer) {
  if (!this->visible)
    return;

  GLuint program = ResourceCache::hudTexturedPolygonProgram;

  glUseProgram(program);

  GLuint textureUniform = glGetUniformLocation(program, "texture0");

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, ResourceCache::GetTexture("digits"));
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

  glDrawArrays(GL_TRIANGLES, 0, 6 * this->ndigits);
  if (glGetError() != GL_NO_ERROR)
    cout << "nw: OpenGL draw error." << endl;

  glDisableVertexAttribArray(coordAttr);
  glDisableVertexAttribArray(texCoordAttr);

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glUseProgram(0);
}

void NumberWidget::Reset() {

}
