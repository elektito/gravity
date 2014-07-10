#version 330

uniform sampler2D texture0;

in VERTEX {
  vec2 coord;
  vec2 tex_coord;
} vertex;

out vec4 output_color;

void main() {
  output_color = texture(texture0, vertex.tex_coord);
}
