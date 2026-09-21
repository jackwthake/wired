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

#define NUM_APPS 20
extern struct app_desc apps_registry[NUM_APPS];


void app_desc_free(struct app_desc *a);

#endif __APPS_H__ // __APP_DESC_H__