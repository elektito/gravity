#version 330

uniform sampler2D texture0;

in VERTEX {
  vec2 coord;
  vec2 tex_coord;
  vec4 color;
} vertex;

out vec4 output_color;

void main() {
  // Flip the y axis in texture coordinates.
  vec2 tex_coord = vec2(vertex.tex_coord.x, 1.0 - vertex.tex_coord.y);

  // Sample the texture and mix the result with the color.
  output_color = texture(texture0, tex_coord) * vertex.color;
}
