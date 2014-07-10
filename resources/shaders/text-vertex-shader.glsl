#version 330

in vec2 coord;
in vec2 tex_coord;
in vec4 color;

out VERTEX {
  vec2 tex_coord;
  vec4 color;
} vertex;

void main() {
  vertex.tex_coord = tex_coord;
  vertex.color = color;
  gl_Position = vec4(coord, 0.0, 1.0);
}
