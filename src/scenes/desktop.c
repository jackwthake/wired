#include "scenes.h"

#include <string.h>

#include "../gfx.h"
#include "fsm.h"

#include "navi.h"

extern state_machine_t main_state;
struct navi_t navi;

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

  for (int y = 0; y < 240; y++) {
    const uint32_t *row = (y * 12 < 240 * 8) ? top      // top 2/3
                        : (y * 12 < 240 * 9) ? mid      // next 1/12
                        : bot;                          // bottom 1/4
    for (int x = 0; x < 320; x++)
      w->framebuff[y * w->w + x] = row[x * 7 / w->w];
  }
}


void desktop_enter(void *n, size_t s) {
  navi_init(&navi, 10);
  fsm_update_internal_state(&main_state, &navi, sizeof(struct navi_t));

  navi_add_window(&navi, (SCREEN_W / 2) - 160, (SCREEN_H / 2) - 120, 320, 240, "test pattern", NULL, 0, draw_test_pattern);
}


void desktop_tick(void *n, size_t s, float dt) {

}


int desktop_render(void *n, size_t s) {
  struct navi_t *navi = (struct navi_t*)n;

  memset(gfx_pixels(), RGB(0, 0, 0), SCREEN_W * SCREEN_H * sizeof(uint32_t));

  navi_update_windows(navi);
  return 1;
}


state_interface_t desktop_scene = {
  desktop_enter,
  desktop_tick,
  desktop_render,
  NULL
};

