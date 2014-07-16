#version 330

in vec2 coord;
in vec2 tex_coord;
in vec2 position;
in float angle;
in float scale_factor;

out VERTEX {
  vec2 coord;
  vec2 tex_coord;
} vertex;

vec2 rotate(in vec2 coord, in float angle, in vec2 origin) {
  mat2 r = mat2(cos(angle), sin(angle), -sin(angle), cos(angle));
  return r * (coord - origin) + origin;
}

vec2 scale(in vec2 coord, in float factor, in vec2 origin) {
  return factor * (coord - origin) + origin;
}

void main() {
  vertex.coord = rotate(position + coord, angle, position);
  vertex.coord = scale(vertex.coord, scale_factor, position);

  vertex.tex_coord = tex_coord;
  gl_Position = vec4(vertex.coord, 0.0, 1.0);
}
