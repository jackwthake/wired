#include "platform.h"

#include "gfx.h"
#include "navi.h"

extern const unsigned int PLATFORM_SCREEN_WIDTH;
extern const unsigned int PLATFORM_SCREEN_HEIGHT;


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


int main(int argc, char* argv[]) {
  struct platform platform;

  if (!platform_init(&platform)) {
    return 1;
  }

  if (!gfx_init()) {
    LOG("main: gfx_init failed");
    platform_shutdown(&platform);
    return 1;
  }

  struct navi_t navi;
  navi_init(&navi, 10);

  navi_add_window(&navi, (SCREEN_W / 2) - 160, (SCREEN_H / 2) - 120, 320, 240, "test pattern", NULL, 0, draw_test_pattern);

  while (platform.running) {
    platform_update(&platform);

    navi_update_windows(&navi);

    gfx_upload();    
    gfx_present(PLATFORM_SCREEN_WIDTH, PLATFORM_SCREEN_HEIGHT, platform.platform_time);
    
    platform_swap_buffers(&platform);

    // sleep for a short duration to avoid busy waiting
    SDL_Delay(16); // ~60 FPS
  }

  platform_shutdown(&platform);
  return 0;
}
