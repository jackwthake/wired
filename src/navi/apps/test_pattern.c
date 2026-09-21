# include "test_pattern.h"

#include "gfx.h"

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