#ifndef __NAVI_H__
#define __NAVI_H__

#include <stdint.h>

#include "apps/apps.h"
#include "input.h"

struct window;
typedef void (*navi_win_callback)(struct window *);

struct window {
  unsigned x, y, w, h;      // content area; the title bar sits above it (y - TITLE_BAR_H)
  char *title;

  uint32_t *framebuff;      // w * h pixels the window's update callback draws into

  void *udata;              // owned by whoever created the window
  unsigned udata_size;

  navi_win_callback update;

  // ---- managed by navi; read-only for apps ----
  unsigned char alive;
  unsigned char focused;
  const struct input *in;   // non-NULL only while this window has focus
  int mx, my;               // mouse in content coordinates (can be outside 0..w / 0..h)

  // ---- apps may set this from inside update() to have navi close them safely ----
  unsigned char close_requested;
};


struct navi_t {
  struct window *open_windows;   // stable storage: slots never move, so window pointers stay valid
  unsigned *order;               // alive slot indices, back to front (last = focused)
  unsigned max_windows;
  unsigned count;                // number of alive windows == number of entries in order[]

  int drag_slot;                 // slot being dragged, or -1
  int drag_dx, drag_dy;          // grab offset inside the window

  uint32_t *close_btn;
  int close_btn_w, close_btn_h;

  uint32_t *cursor;
  int cursor_w, cursor_h;

  int last_icon; double last_click_time; int selected_icon;
};


void navi_init(struct navi_t *navi, unsigned max_open);
void navi_free(struct navi_t *navi);

// New windows appear on top and focused. Returns NULL if full.
struct window *navi_add_window(struct navi_t *navi, unsigned x, unsigned y, unsigned w, unsigned h,
                               char *title, void *udata, unsigned udata_size, navi_win_callback updatefn);
void navi_launch(struct navi_t *navi, struct app_desc *a);

// Frees the window's framebuffer and title (not udata). The pointer is dead afterwards.
void navi_close_window(struct navi_t *navi, struct window *win);

// Once per frame: routes input (focus, drag, close button), then runs each
// window's update callback and draws back to front into gfx_pixels().
void navi_update_windows(struct navi_t *navi, struct input *in);
void navi_draw_cursor(struct navi_t *navi, const struct input *in);


// Drawing helpers. The *_fb variants draw into any buffer (e.g. a window's own
// framebuff); the older names draw into the full-screen buffer.
void draw_bitmap_to_framebuffer(uint32_t *framebuffer, int fb_w, int fb_h, uint32_t *bitmap, int bmp_w, int bmp_h, int x_offset, int y_offset);
void draw_string_to_framebuffer(uint32_t *framebuffer, const char *str, int x_offset, int y_offset, int *next_char_x, int *next_char_y, uint32_t color);
void draw_string_fb(uint32_t *fb, int fb_w, int fb_h, const char *str, int x, int y, int *next_x, int *next_y, uint32_t color);

#endif // __NAVI_H__