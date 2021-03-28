#ifndef _GRAVITY_MESH_HH_
#define _GRAVITY_MESH_HH_

#include "glew.h"
#include "resource-cache.hh"
#include <box2d/box2d.h>

class Mesh {
protected:
  GLuint vbo;
  ResourceCache::Texture texture;
  int vertexCount;

  struct {
    float r;
    float g;
    float b;
    float a;
  } color;

public:
  Mesh(const GLfloat *vertexData, int n, ResourceCache::Texture texture);
  ~Mesh();

  void SetColor(float r, float g, float b, float a);
  void Draw(const b2Vec2 &pos, float angle, float scale_factor=1.0f) const;
};

#endif /* _GRAVITY_MESH_HH_ */
