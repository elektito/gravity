#ifndef _GRAVITY_MESH_HH_
#define _GRAVITY_MESH_HH_

#include <GL/glew.h>
#include <Box2D/Box2D.h>

class Mesh {
protected:
  GLuint vbo;
  GLuint texture;
  int vertexCount;
  GLuint program;

public:
  Mesh(const GLfloat *vertexData, int n, GLuint texture, bool hud=false);
  ~Mesh();

  void Draw(const b2Vec2 &pos, float32 angle, float32 scale_factor=1.0f) const;
};

#endif /* _GRAVITY_MESH_HH_ */
