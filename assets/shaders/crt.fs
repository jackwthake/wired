#ifdef GL_FRAGMENT_PRECISION_HIGH
  precision highp float;
#else
  precision mediump float;
#endif

uniform sampler2D uTex;
uniform vec2 uSrc;
uniform float uTime;
varying vec2 vUV;
uniform float uAberation;
uniform float uFlipY;

// Pseudo-random noise generator
float rand(vec2 co) {
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main() {
  // Screen curvature (barrel distortion)
  vec2 tc = vUV - 0.5;
  float dist = dot(tc, tc);
  tc *= 1.0 + dist * 0.1;
  tc += 0.5;

  // Out-of-bounds check (black border)
  if (tc.x < 0.0 || tc.x > 1.0 || tc.y < 0.0 || tc.y > 1.0) {
    gl_FragColor = vec4(0.05, 0.05, 0.05, 1.0);
  } else {
    float safe_time = mod(uTime, 100.0); // Prevent float precision errors

    // Head switching artifact: bottom lines skew sideways, growing toward the edge
    float line = floor(tc.y * uSrc.y);              // one random value per source scanline
    float hs   = smoothstep(0.975, 1.0, tc.y);      // 0 above the zone, 1 at the bottom edge
    float n    = rand(vec2(line, floor(safe_time * 30.0))) - 0.5;   // -0.5..0.5, redrawn 30x/sec
    tc.x += n * 0.12 * hs;                          // random left/right shift per line
    float valid = step(0.0, tc.x) * step(tc.x, 1.0);

    // Chromatic Aberration (Now samples the newly distorted coordinates)
    float r = texture2D(uTex, vec2(tc.x - uAberation, tc.y)).r;
    float g = texture2D(uTex, tc).g;
    float b = texture2D(uTex, vec2(tc.x + uAberation, tc.y)).b;
    
    vec4 cta = vec4(r, g, b, 1.0);
    cta.rgb *= valid;

    // Base horizontal scanlines
    float scanline = sin(tc.y * uSrc.y * 3.14159) * 0.12;
    cta.rgb -= scanline;

    // Discrete Rolling Interference Bars
    float bar_coord = fract((tc.y - uTime * 0.05) * 0.75);
    float bar_mask = step(bar_coord, 0.1); 
    cta.rgb -= bar_mask * 0.05; 

    gl_FragColor = cta;
  }
}
