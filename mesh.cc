#include "mesh.hh"
#include "resource-cache.hh"

#include <iostream>

Mesh::Mesh(const GLfloat *vertexData, int n, GLuint texture) :
  vertexCount(n),
  texture(texture),
  color({1.0f, 1.0f, 1.0f, 1.0f})
{
  glGenBuffers(1, &this->vbo);

  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
  glBufferData(GL_ARRAY_BUFFER, n * 4 * sizeof(GLfloat), vertexData, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Mesh::~Mesh() {
  glDeleteBuffers(1, &this->vbo);
}

void Mesh::SetColor(float r, float g, float b, float a) {
  this->color = {r, g, b, a};
}

void Mesh::Draw(const b2Vec2 &pos, float angle, float scale_factor) const {
  GLuint program = ResourceCache::texturedPolygonProgram;
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
  GLint angleAttr = glGetAttribLocation(program, "angle");
  GLint scaleAttr = glGetAttribLocation(program, "scale_factor");
  GLint colorAttr = glGetAttribLocation(program, "color");

  glEnableVertexAttribArray(coordAttr);
  glEnableVertexAttribArray(texCoordAttr);

  glVertexAttribPointer(coordAttr, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*) 0);
  glVertexAttribPointer(texCoordAttr, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*) (2 * sizeof(GLfloat)));
  glVertexAttrib2f(positionAttr, pos.x, pos.y);
  glVertexAttrib1f(angleAttr, angle);
  glVertexAttrib1f(scaleAttr, scale_factor);
  glVertexAttrib4f(colorAttr, this->color.r, this->color.g, this->color.b, this->color.a);

  glDrawArrays(GL_TRIANGLES, 0, this->vertexCount);
  if (glGetError() != GL_NO_ERROR)
    cout << "mesh: OpenGL draw error." << endl;

  glDisableVertexAttribArray(coordAttr);
  glDisableVertexAttribArray(texCoordAttr);

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glUseProgram(0);
}
