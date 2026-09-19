#ifdef GL_FRAGMENT_PRECISION_HIGH
  precision highp float;
#else
  precision mediump float;
#endif

uniform float uTime;
varying vec2 vUV;

float b2(vec2 p) { return fract(p.x * 0.5 + p.y * p.y * 0.75); }

float bayer4(vec2 p) {
  return b2(mod(p, 2.0)) + 0.25 * b2(mod(floor(p * 0.5), 2.0));
}

void main() {
  float g = 0.10 + 0.25 * (1.0 - vUV.y);
  g += 0.03 * sin(uTime * 0.5 + vUV.y * 6.0);
  vec3 col = vec3(g * 0.35, g * 0.55, g);

  // quantize to 4 levels per channel with ordered dither
  float d = bayer4(gl_FragCoord.xy) - 0.5;
  col = floor(col * 3.0 + 0.5 + d * 0.9) / 3.0;
  gl_FragColor = vec4(col, 1.0);
}