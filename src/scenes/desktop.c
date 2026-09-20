#include "scenes.h"

#include <stdlib.h>
#include <string.h>

#include "../gfx.h"
#include "../platform.h"
#include "fsm.h"

#include "navi.h"

extern state_machine_t main_state;
extern struct platform platform;

static struct navi_t navi;
static uint32_t *bg;


void draw_test_pattern(struct window *w) {
  static const uint32_t top[7] = {           // 75% bars: gray, yellow, cyan, green, magenta, red, blue
    RGB(191,191,191), RGB(191,191,0), RGB(0,191,191), RGB(0,191,0),
    RGB(191,0,191),   RGB(191,0,0),   RGB(0,0,191)
  };
  static const uint32_t mid[7] = {           // thin reverse-blue strip
    RGB(0,0,191), RGB(0,0,0), RGB(191,0,191), RGB(0,0,0),
    RGB(0,191,191), RGB(0,0,0), RGB(191,191,191)
  };
  static const uint32_t bot[7] = {           // -I, white, +Q, black, then near-black steps
    RGB(0,33,76), RGB(255,255,255), RGB(50,0,106), RGB(0,0,0),
    RGB(0,0,0),   RGB(12,12,12),    RGB(0,0,0)
  };

  for (int y = 0; y < w->h; y++) {
    const uint32_t *row = (y * 12 < w->h * 8) ? top      // top 2/3
                        : (y * 12 < w->h * 9) ? mid      // next 1/12
                        : bot;                          // bottom 1/4
    for (int x = 0; x < w->w; x++)
      w->framebuff[y * w->w + x] = row[x * 7 / w->w];
  }
}


void desktop_enter(void *n, size_t s) {
  navi_init(&navi, 10);
  fsm_update_internal_state(&main_state, &navi, sizeof(struct navi_t));

  int width, height;
  bg = convert_bmp_to_framebuffer("bg.bmp", &width, &height);

  navi_add_window(&navi, 50, 50, 320, 240, "test pattern", NULL, 0, draw_test_pattern);
  navi_add_window(&navi, 400, 400, 150, 150, "test pattern 2", NULL, 0, draw_test_pattern);
}


void desktop_tick(void *n, size_t s, float dt) {

}


int desktop_render(void *n, size_t s) {
  struct navi_t *navi = (struct navi_t*)n;

  // clear screen
  draw_bitmap_to_framebuffer(gfx_pixels(), SCREEN_W, SCREEN_H, bg, SCREEN_W, SCREEN_H, 0, 0);

  navi_update_windows(navi, &platform.input);
  navi_draw_cursor(navi, &platform.input);

  return 1;
}


void desktop_exit(void *n, size_t s) {
  navi_free((struct navi_t *)n);
  free(bg);
}


state_interface_t desktop_scene = {
  desktop_enter,
  desktop_tick,
  desktop_render,
  desktop_exit
};

