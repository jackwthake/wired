#include "navi.h"

#include <stdlib.h>
#include <string.h>

#include "platform.h"
#include "gfx.h"

#include "font.inc"

#include "navi/apps/apps.h"

#define TITLE_BAR_H 20
#define GLYPH_WIDTH 8
#define GLYPH_HEIGHT 16

#define COL_TITLE_FOCUSED    RGB(50, 50, 100)
#define COL_TITLE_UNFOCUSED  RGB(30, 30, 52)
#define COL_TEXT_FOCUSED     RGB(200, 200, 200)
#define COL_TEXT_UNFOCUSED   RGB(110, 110, 120)

extern struct platform *platform;


// ------------------------------------------------------------- drawing

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


void draw_bitmap_to_framebuffer(uint32_t *framebuffer, int fb_w, int fb_h, uint32_t *bitmap, int bmp_w, int bmp_h, int x_offset, int y_offset) {
  for (int y = 0; y < bmp_h; y++) {
    for (int x = 0; x < bmp_w; x++) {
      uint32_t color = bitmap[y * bmp_w + x];
      if (color != RGB(255, 0, 255)) { // skip transparent pixels
        int fb_x = x + x_offset;
        int fb_y = y + y_offset;
        if (fb_x >= 0 && fb_x < fb_w && fb_y >= 0 && fb_y < fb_h) {
          framebuffer[fb_y * fb_w + fb_x] = color;
        }
      }
    }
  }
}


static void draw_glyph(uint32_t *fb, int fb_w, int fb_h, unsigned char c, int x0, int y0, uint32_t color) {
  for (int y = 0; y < GLYPH_HEIGHT; y++) {
    int fy = y0 + y;
    if (fy < 0 || fy >= fb_h) continue;

    unsigned char row = glyph_bitmaps[c][y];
    for (int x = 0; x < GLYPH_WIDTH; x++) {
      int fx = x0 + x;
      if (fx < 0 || fx >= fb_w) continue;

      if (row & (1 << (7 - x))) {
        fb[fy * fb_w + fx] = color;
      }
    }
  }
}


// Draws into any buffer, so a window can render text into its own framebuff.
void draw_string_fb(uint32_t *fb, int fb_w, int fb_h, const char *str, int x, int y, int *next_x, int *next_y, uint32_t color) {
  int x_start = x;

  for (; *str; ++str) {
    if (*str == '\n') {
      y += GLYPH_HEIGHT;
      x = x_start;
    } else if (*str == '\t') {
      x += GLYPH_WIDTH * 2;
    } else {
      draw_glyph(fb, fb_w, fb_h, (unsigned char)*str, x, y, color);
      x += GLYPH_WIDTH;
    }
  }

  if (next_x) *next_x = x;
  if (next_y) *next_y = y;
}


// full-screen version, kept so existing callers still compile
void draw_string_to_framebuffer(uint32_t *framebuffer, const char *str, int x_offset, int y_offset, int *next_char_x, int *next_char_y, uint32_t color) {
  draw_string_fb(framebuffer, SCREEN_W, SCREEN_H, str, x_offset, y_offset, next_char_x, next_char_y, color);
}


// ------------------------------------------------------------- helpers

static int title_top(const struct window *w) {
  return (int)w->y - TITLE_BAR_H;
}


static int point_in(int px, int py, int x, int y, int w, int h) {
  return px >= x && px < x + w && py >= y && py < y + h;
}


// keep the whole window (title bar included) on the machine's screen
static void clamp_window(struct window *w) {
  int x = (int)w->x, y = (int)w->y;
  int max_x = SCREEN_W - (int)w->w;
  int max_y = SCREEN_H - (int)w->h;

  if (x > max_x) x = max_x;
  if (x < 0) x = 0;
  if (y > max_y) y = max_y;
  if (y < TITLE_BAR_H) y = TITLE_BAR_H;

  w->x = (unsigned)x;
  w->y = (unsigned)y;
}


static void close_slot(struct navi_t *navi, unsigned slot) {
  struct window *w = &navi->open_windows[slot];
  if (!w->alive) return;

  free(w->framebuff);
  free(w->title);
  memset(w, 0, sizeof *w);   // also clears alive; udata belongs to the caller

  unsigned i = 0;
  while (i < navi->count && navi->order[i] != slot) ++i;
  if (i < navi->count) {
    memmove(&navi->order[i], &navi->order[i + 1], (navi->count - i - 1) * sizeof(unsigned));
    --navi->count;
  }

  if (navi->drag_slot == (int)slot) navi->drag_slot = -1;
}


// move order[i] to the top of the stack (top = last = focused)
static void focus_index(struct navi_t *navi, unsigned i) {
  unsigned slot = navi->order[i];
  memmove(&navi->order[i], &navi->order[i + 1], (navi->count - i - 1) * sizeof(unsigned));
  navi->order[navi->count - 1] = slot;
}


// ------------------------------------------------------------- lifecycle

void navi_init(struct navi_t *navi, unsigned max_open) {
  if (!navi) return;

  navi->open_windows = calloc(max_open, sizeof(struct window));
  navi->order = calloc(max_open, sizeof(unsigned));
  navi->max_windows = max_open;
  navi->count = 0;

  navi->drag_slot = -1;
  navi->drag_dx = navi->drag_dy = 0;

  navi->close_btn_w = navi->close_btn_h = 0;
  navi->close_btn = convert_bmp_to_framebuffer("close.bmp", &navi->close_btn_w, &navi->close_btn_h);

  navi->cursor_w = navi->cursor_h = 0;
  navi->cursor = convert_bmp_to_framebuffer("cursor.bmp", &navi->cursor_w, &navi->cursor_h);

  navi->last_icon = navi->selected_icon = -1;
  navi->last_click_time = platform->platform_time;

  // load program icons
  unsigned next_icon_x = 10;
  unsigned next_icon_y = 10;
  static const unsigned icon_vertical_padding = 20;
  static const unsigned icon_horizontal_padding = 5;

  for (unsigned i = 0; i < NUM_APPS; ++i) {
    struct app_desc *app = &apps_registry[i]; 
    app->icon_px = convert_bmp_to_framebuffer(app->icon, &app->icon_w, &app->icon_h);
    app->icon_x = next_icon_x;
    app->icon_y = next_icon_y;

    next_icon_y += app->icon_h + icon_vertical_padding;
    if (next_icon_y > SCREEN_H * 0.8) {
      next_icon_y = 10;
      next_icon_x += app->icon_w + icon_horizontal_padding;
    }
  }
}


void navi_free(struct navi_t *navi) {
  if (!navi) return;

  for (unsigned i = 0; i < navi->max_windows; ++i) {
    struct window *w = &navi->open_windows[i];
    if (w->alive) {
      free(w->framebuff);
      free(w->title);
    }
  }

  free(navi->open_windows);
  free(navi->order);
  free(navi->close_btn);
  free(navi->cursor);

  for (unsigned i = 0; i < NUM_APPS; ++i) {
    struct app_desc *app = &apps_registry[i]; 
    app_desc_free(app);
  }

  memset(navi, 0, sizeof *navi);
}


struct window *navi_add_window(struct navi_t *navi, unsigned x, unsigned y, unsigned w, unsigned h,
                               char *title, void *udata, unsigned udata_size, navi_win_callback updatefn) {
  if (!navi || navi->count >= navi->max_windows) return NULL;

  // first free slot (slots are reused after a window closes)
  unsigned slot = 0;
  while (slot < navi->max_windows && navi->open_windows[slot].alive) ++slot;
  if (slot == navi->max_windows) return NULL;

  struct window *win = &navi->open_windows[slot];
  memset(win, 0, sizeof *win);

  win->x = x;
  win->y = y;
  win->w = w;
  win->h = h;

  win->framebuff = calloc((size_t)w * h, sizeof(uint32_t));
  if (!win->framebuff) return NULL;

  size_t title_size = strlen(title) + 1;
  win->title = malloc(title_size);
  if (!win->title) {
    free(win->framebuff);
    win->framebuff = NULL;
    return NULL;
  }
  memcpy(win->title, title, title_size);

  win->udata = udata;
  win->udata_size = udata_size;
  win->update = updatefn;
  win->alive = 1;

  clamp_window(win);

  navi->order[navi->count++] = slot;   // new windows open on top
  return win;
}


void navi_launch(struct navi_t *navi, struct app_desc *a) {
  void *udata = calloc(1, a->state_size);
  struct window * w = navi_add_window(navi, 100, 100, a->w, a->h, a->name, udata, a->state_size, a->update);

  if (a->init) {
    a->init(w);
  }
}


void navi_close_window(struct navi_t *navi, struct window *win) {
  if (!navi || !win) return;
  if (win->udata) {
    free(win->udata);
    win->udata_size = 0;
  }

  close_slot(navi, (unsigned)(win - navi->open_windows));
}


// ------------------------------------------------------------- input

static int icon_at(struct navi_t *navi, int x, int y) {
  (void)navi;
  for (unsigned i = 0; i < NUM_APPS; ++i) {
    struct app_desc *app = &apps_registry[i];

    if (x >= (int)app->icon_x && x < (int)(app->icon_x + app->icon_w) &&
        y >= (int)app->icon_y && y < (int)(app->icon_y + app->icon_h + 16)) {
      return (int)i;
    }
  }

  return -1;
}

static void handle_mouse(struct navi_t *navi, const struct input *in) {
  // dragging
  if (navi->drag_slot >= 0) {
    struct window *w = &navi->open_windows[navi->drag_slot];

    if (in->mouse_down && in->mouse_valid) {
      int nx = in->mouse_x - navi->drag_dx;
      int ny = in->mouse_y - navi->drag_dy;
      w->x = nx < 0 ? 0 : (unsigned)nx;
      w->y = ny < 0 ? 0 : (unsigned)ny;
      clamp_window(w);
    }

    if (!in->mouse_down) navi->drag_slot = -1;
  }

  if (!in->mouse_pressed || !in->mouse_valid) return;

  // click: hit-test front to back, first hit wins
  for (int i = (int)navi->count - 1; i >= 0; --i) {
    unsigned slot = navi->order[i];
    struct window *w = &navi->open_windows[slot];
    int tt = title_top(w);

    if (!point_in(in->mouse_x, in->mouse_y, (int)w->x, tt, (int)w->w, (int)w->h + TITLE_BAR_H)) {
      continue;
    }

    // close button
    int bx = (int)(w->x + w->w) - (navi->close_btn_w + 2);
    int by = tt + 2;
    if (point_in(in->mouse_x, in->mouse_y, bx, by, navi->close_btn_w, navi->close_btn_h)) {
      close_slot(navi, slot);
      return;
    }

    focus_index(navi, (unsigned)i);

    // title bar: start a drag
    if (in->mouse_y < (int)w->y) {
      navi->drag_slot = (int)slot;
      navi->drag_dx = in->mouse_x - (int)w->x;
      navi->drag_dy = in->mouse_y - (int)w->y;
    }

    // A hit on a window consumes the click; do not also treat the same
    // location as a desktop icon activation.
    return;
  }

  // no window was hit: this click is on the desktop
  int hit = icon_at(navi, in->mouse_x, in->mouse_y);

  if (hit >= 0) {
    if (hit == navi->last_icon && in->time - navi->last_click_time < 0.4) {
      navi_launch(navi, &apps_registry[hit]);   // second click, same icon, fast enough
      LOG("navi: launching: %s", apps_registry[hit].name);
      navi->last_icon = -1;                     // so a third click doesn't launch again
    } else {
      navi->last_icon = hit;                    // first click: remember it
      navi->last_click_time = in->time;
    }
    navi->selected_icon = hit;
  } else {
    navi->last_icon = navi->selected_icon = -1;   // clicked empty desktop
  }
}


// ------------------------------------------------------------- drawing + update

static void draw_window(struct navi_t *navi, struct window *win, uint32_t *pixels) {
  int tt = title_top(win);

  draw_rect(pixels, win->x, (unsigned)tt, win->w, TITLE_BAR_H,
            win->focused ? COL_TITLE_FOCUSED : COL_TITLE_UNFOCUSED);

  draw_string_to_framebuffer(pixels, win->title, (int)win->x + 2, tt + 2, NULL, NULL,
                             win->focused ? COL_TEXT_FOCUSED : COL_TEXT_UNFOCUSED);

  if (navi->close_btn) {
    draw_bitmap_to_framebuffer(pixels, SCREEN_W, SCREEN_H, navi->close_btn,
                               navi->close_btn_w, navi->close_btn_h,
                               (int)(win->x + win->w) - (navi->close_btn_w + 2), tt + 2);
  }

  for (unsigned y = 0; y < win->h; ++y) {
    unsigned py = y + win->y;
    if (py >= SCREEN_H) break;

    for (unsigned x = 0; x < win->w; ++x) {
      unsigned px = x + win->x;
      if (px >= SCREEN_W) break;

      pixels[py * SCREEN_W + px] = win->framebuff[y * win->w + x];
    }
  }
}


void navi_draw_cursor(struct navi_t *navi, const struct input *in) {
  if (!navi->cursor || !in || !in->mouse_valid) return;
  draw_bitmap_to_framebuffer(gfx_pixels(), SCREEN_W, SCREEN_H, navi->cursor,
                             navi->cursor_w, navi->cursor_h, in->mouse_x, in->mouse_y);
}


void navi_update_windows(struct navi_t *navi, struct input *in) {
  if (!navi) return;

  if (in) handle_mouse(navi, in);

  uint32_t *pixels = gfx_pixels();

  // back to front; the last entry in order[] is the focused window
  for (unsigned i = 0; i < navi->count; ++i) {
    struct window *win = &navi->open_windows[navi->order[i]];
    int focused = (i == navi->count - 1);

    win->focused = (unsigned char)focused;
    win->in = (focused && in) ? in : NULL;
    win->mx = in ? in->mouse_x - (int)win->x : 0;
    win->my = in ? in->mouse_y - (int)win->y : 0;

    if (win->update) win->update(win);

    draw_window(navi, win, pixels);
  }

  // windows that asked to close during update()
  for (int i = (int)navi->count - 1; i >= 0; --i) {
    unsigned slot = navi->order[i];
    if (navi->open_windows[slot].close_requested) close_slot(navi, slot);
  }

  in->time = platform->platform_time;
}