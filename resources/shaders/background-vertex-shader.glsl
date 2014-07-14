#version 330

in vec2 coord;
in vec2 tex_coord;

out VERTEX {
  vec2 tex_coord;
} vertex;

void main() {
  vertex.tex_coord = tex_coord;
  gl_Position = vec4(coord, 0.0, 1.0);
}
