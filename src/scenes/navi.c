#include "navi.h"

#include <stdlib.h>
#include <string.h>

#include "gfx.h"

#include "font.inc"

#define TITLE_BAR_H 20


static void draw_rect(uint32_t *pixels, unsigned x, unsigned y, unsigned w, unsigned h, uint32_t color) {
  if (!pixels || w == 0 || h == 0) {
    return;
  }

  for (unsigned dy = 0; dy < h; ++dy) {
    unsigned py = y + dy;
    if (py >= SCREEN_H) break;

    for (unsigned dx = 0; dx < w; ++dx) {
      unsigned px = x + dx;
      if (px >= SCREEN_W) break;

      pixels[py * SCREEN_W + px] = color;
    }
  }
}


// draw from 8x16 font bitmap to framebuffer at specified position
#define GLYPH_WIDTH 8
#define GLYPH_HEIGHT 16

static void draw_glyph_to_framebuffer(uint32_t *framebuffer, const char c, int x_offset, int y_offset, uint32_t color) {
  if (c < 0 || c > 255) return; // Only handle ASCII range

  for (int y = 0; y < GLYPH_HEIGHT; y++) {
    unsigned char row = glyph_bitmaps[(unsigned char)c][y];
    for (int x = 0; x < GLYPH_WIDTH; x++) {
      if (row & (1 << (7 - x))) { // Check if pixel is set
        int fb_x = x + x_offset;
        int fb_y = y + y_offset;
        if (fb_x >= 0 && fb_x < SCREEN_W && fb_y >= 0 && fb_y < SCREEN_H) {
          framebuffer[fb_y * SCREEN_W + fb_x] = color;
        }
      }
    }
  }
}


void draw_string_to_framebuffer(uint32_t *framebuffer, const char *str, int x_offset, int y_offset, int *next_char_x, int *next_char_y, uint32_t color) {
  int original_x_offset = x_offset; // Store the original x_offset for line breaks
  unsigned str_len = strlen(str);

  while (*str && str_len > 0) {
    if (*str == '\n') {
      y_offset += GLYPH_HEIGHT; // Move to the next line
      x_offset = original_x_offset; // Reset x position
      if (next_char_x) *next_char_x = x_offset;
      if (next_char_y) *next_char_y = y_offset;
    } else if (*str == '\t'){
      x_offset += GLYPH_WIDTH * 2; // Move to the next tab
      if (next_char_x) *next_char_x = x_offset;
      if (next_char_y) *next_char_y = y_offset;
    } else {
      draw_glyph_to_framebuffer(framebuffer, *str, x_offset, y_offset, color);
      x_offset += GLYPH_WIDTH; // Move to the next character position
      if (next_char_x) *next_char_x = x_offset;
      if (next_char_y) *next_char_y = y_offset;
    }
    
    str++;
    str_len--;
  }
}


void navi_init(struct navi_t *navi, unsigned max_open) {
  if (!navi) return;

  navi->open_windows = calloc(max_open, sizeof(struct window));
  navi->max_windows = max_open;
  navi->next_idx = 0;
}


struct window *navi_add_window(struct navi_t *navi, unsigned x, unsigned y, unsigned w, unsigned h, 
                         char *title, void *udata, unsigned udata_size, navi_win_callback updatefn) {
  if (!navi || navi->next_idx >= navi->max_windows) return NULL;

  struct window *win = &navi->open_windows[navi->next_idx];
  win->x = x;
  win->y = y;
  win->w = w;
  win->h = h;

  win->framebuff = calloc(w * h, sizeof(uint32_t));

  unsigned title_size = strlen(title) + 1;
  win->title = calloc(title_size, sizeof(char));
  memcpy(win->title, title, title_size);

  win->udata = udata;
  win->udata_size = udata_size;

  win->update = updatefn;
  ++navi->next_idx;
  return win;
}


void draw_window(struct window *win, uint32_t *pixels) {
  unsigned title_y = win->y >= 10 ? win->y - TITLE_BAR_H : 0;
  draw_rect(pixels, win->x, title_y, win->w, TITLE_BAR_H, RGB(50, 50, 100));

  draw_string_to_framebuffer(pixels, win->title, win->x + 2, title_y + 2, NULL, NULL, RGB(200, 200, 200));

  for (unsigned y = 0; y < win->h; ++y) {
    for (unsigned x = 0; x < win->w; ++x) {
      unsigned offset_x = x + win->x;
      unsigned offset_y = y + win->y;

      if (offset_x < SCREEN_W && offset_y < SCREEN_H) {
        pixels[offset_y * SCREEN_W + offset_x] = win->framebuff[y * win->w + x];
      }
    }
  }
}


void navi_update_windows(struct navi_t *navi) {
  if (!navi) return;

  uint32_t *pixels = gfx_pixels();

  for (unsigned i = 0; i < navi->next_idx; ++i) {
    struct window *win = &navi->open_windows[i];

    if (!win || !win->framebuff || !win->update) {
      continue;
    }

    win->update(win);
    if (win->y < TITLE_BAR_H) win->y = TITLE_BAR_H;

    draw_window(win, pixels);
  }
}
