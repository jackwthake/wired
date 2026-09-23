#include "scenes.h"

#include <stdlib.h>
#include <string.h>

#include "gfx.h"
#include "platform.h"
#include "fsm.h"

#include "navi/navi.h"
#include "navi/apps/apps.h"

extern state_machine_t main_state;
extern struct platform platform;

static struct navi_t navi;
static uint32_t *bg;


void desktop_enter(void *n, size_t s) {
  navi_init(&navi, 10);
  fsm_update_internal_state(&main_state, &navi, sizeof(struct navi_t));

  int width, height;
  bg = convert_bmp_to_framebuffer("bg.bmp", &width, &height);
}


void desktop_tick(void *n, size_t s, float dt) {
  for (int i = 0; i < platform.input.key_count; ++i) {
    switch (platform.input.keys[i]) {
      case NK_ESCAPE:
        platform.running = false;
        break;
      default: break;
    }
  }
}


void draw_desktop_icons(void) {
  for (unsigned i = 0; i < NUM_APPS; ++i) {
    struct app_desc *app = &apps_registry[i];
    uint32_t *pixels = gfx_pixels();

    draw_bitmap_to_framebuffer(pixels, SCREEN_W, SCREEN_H, app->icon_px, app->icon_w, app->icon_h, app->icon_x, app->icon_y);
    draw_string_to_framebuffer(pixels, app->name, app->icon_x, app->icon_y + app->icon_h + 2, NULL, NULL, RGB(200, 200, 200));
  }
}


int desktop_render(void *n, size_t s) {
  struct navi_t *navi = (struct navi_t*)n;

  // clear screen
  draw_bitmap_to_framebuffer(gfx_pixels(), SCREEN_W, SCREEN_H, bg, SCREEN_W, SCREEN_H, 0, 0);

  draw_desktop_icons();

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

