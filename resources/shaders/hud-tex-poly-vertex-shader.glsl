#version 330

uniform vec2 resolution;

const int LEFT = 1;
const int CENTER = 2;
const int RIGHT = 3;
const int BOTTOM = 1;
const int TOP = 3;

in vec2 coord;
in vec2 tex_coord;

/// The position of the instance, with x and y expressed as ratios of
/// screen width and height.
in vec2 position;
in int xalign; // 1=left, 2=center, 3=right
in int yalign; // 1=bottom, 2=center, 3=center

/// Instance height in units of screen height.
in float height;

/// Instance width in units of screen height.
in float width;

out VERTEX {
  vec2 coord;
  vec2 tex_coord;
} vertex;

void main() {
  float w = width * (resolution.y / resolution.x);

  if (xalign == LEFT) {
    vertex.coord.x = position.x * 2.0 - 1.0;
  }
  else if (xalign == CENTER) {
    vertex.coord.x = -w + position.x * 2.0;
  }
  else if (xalign == RIGHT) {
    vertex.coord.x = (1.0 - w + position.x) * 2.0 - 1.0;
  }

  if (yalign == BOTTOM) {
    vertex.coord.y = position.y * 2.0 - 1.0;
  }
  else if (yalign == CENTER) {
    vertex.coord.y = -height + position.y;
  }
  else if (yalign == TOP) {
    vertex.coord.y = (1.0 - height + position.y) * 2.0 - 1.0;
  }

  vertex.coord += 2.0 * coord * vec2(w, height);

  vertex.tex_coord = tex_coord;
  gl_Position = vec4(vertex.coord, 0.0, 1.0);
}
