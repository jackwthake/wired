attribute vec2 aPos;
uniform float uFlipY;
varying vec2 vUV;

void main() {
  vUV = vec2(aPos.x * 0.5 + 0.5, mix(aPos.y * 0.5 + 0.5, 0.5 - aPos.y * 0.5, uFlipY));
  gl_Position = vec4(aPos, 0.0, 1.0);
}