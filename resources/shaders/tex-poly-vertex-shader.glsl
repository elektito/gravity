#version 330

uniform vec2 resolution;
uniform vec2 camera_pos;
uniform float ppm;

in vec2 coord;
in vec2 tex_coord;
in vec2 position;
in float angle;

out VERTEX {
  vec2 coord;
  vec2 tex_coord;
} vertex;

vec2 rotate(in vec2 coord, float angle, vec2 origin) {
  mat2 r = mat2(cos(angle), sin(angle), -sin(angle), cos(angle));
  return r * (coord - origin) + origin;
}

void main() {
  vertex.coord = rotate(position + coord, angle, position);
  vertex.coord -= camera_pos;
  vertex.coord *= ppm;
  vertex.coord = vertex.coord / resolution * 2.0 - 1.0;

  vertex.tex_coord = tex_coord;
  gl_Position = vec4(vertex.coord, 0.0, 1.0);
}
