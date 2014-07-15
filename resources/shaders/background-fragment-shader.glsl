#version 330

uniform sampler2D texture0;

in VERTEX {
  vec2 tex_coord;
} vertex;

out vec4 output_color;

// The following two routines are taken from here:
// http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
vec3 rgb2hsv(vec3 c) {
  vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
  vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
  vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

  float d = q.x - min(q.w, q.y);
  float e = 1.0e-10;
  return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c) {
  vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
  vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
  return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
  output_color = texture(texture0, vertex.tex_coord);

  // Make the color darker. It's much better, performance-wise, to
  // bake this into the texture itself, but we're gonna do it here for
  // now, just for kicks!
  vec3 hsv = rgb2hsv(output_color.rgb);
  hsv.z *= 0.5; // The third component is brightness.
  output_color.rgb = hsv2rgb(hsv);
}
