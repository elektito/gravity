#version 330

uniform sampler2D texture0;

in VERTEX {
  vec2 tex_coord;
  vec4 color;
} vertex;

out vec4 output_color;

void main() {
  output_color = vec4(vertex.color.rgb,
                      texture(texture0, vertex.tex_coord).a * vertex.color.a);
}
