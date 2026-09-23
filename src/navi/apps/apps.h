#ifndef __APPS_H__
#define __APPS_H__

#include <stdint.h>

struct window;

struct app_desc {
  const char *name;            // title bar + icon label
  const char *icon;            // "icons/testpattern.bmp"
  unsigned w, h;               // default content size
  unsigned state_size;         // navi callocs this into win->udata
  void (*init)(struct window *win);      // optional
  void (*update)(struct window *win);
  void (*cleanup)(struct window *win);   // optional

  // filled in by navi_init:
  uint32_t *icon_px, icon_w, icon_h;
  unsigned icon_x, icon_y;
};

#define NUM_APPS 5
extern struct app_desc apps_registry[NUM_APPS];


void app_clear(struct window *win, uint32_t color);
void app_desc_free(struct app_desc *a);


// Programs

extern void files_init(struct window *);
extern void files_update(struct window *);
extern void files_close(struct window *);

extern void notes_init(struct window *);
extern void notes_update(struct window *);
extern void notes_close(struct window *);

extern void terminal_init(struct window *);
extern void terminal_update(struct window *);
extern void terminal_close(struct window *);

extern void draw_test_pattern(struct window *);

#endif // __APP_DESC_H__