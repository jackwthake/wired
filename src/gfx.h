#ifndef __GFX_H__
#define __GFX_H__

// Internal resolution of the fictional machine's "monitor".
// Everything the game draws lives in this space; the CRT pass scales it up.
#define SCREEN_W 640
#define SCREEN_H 480

#include <stdint.h>

// Byte order matches GL_RGBA / GL_UNSIGNED_BYTE on little-endian
#define RGB(r, g, b) ((uint32_t)(r) | ((uint32_t)(g) << 8) | ((uint32_t)(b) << 16) | 0xFF000000u)

// CPU path: write pixels[y * SCREEN_W + x] (y = 0 is the TOP row), then
// gfx_upload() and gfx_present(). Skip gfx_begin_screen / gfx_draw_startup.
uint32_t *gfx_pixels(void);
void      gfx_upload(void);

int  gfx_init(void);      // call once after the GLES2 context is current
void gfx_shutdown(void);

// Per frame:
//   gfx_begin_screen();          bind the low-res target
//   gfx_draw_startup(t);         (later: desktop, windows, terminal...)
//   gfx_present(win_w, win_h, t) CRT pass to the real window
void gfx_begin_screen(void);
void gfx_draw_startup(float t);
void gfx_present(int win_w, int win_h, float t);

int gfx_window_to_screen(int wx, int wy, int win_w, int win_h, int *sx, int *sy);

#endif // __GFX_H__