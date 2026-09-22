#include "navi/navi.h"
#include "gfx.h"

#include <stdio.h>

static char *cwd = "/home/";


void terminal_update(struct window *win) {
  static char prompt[21];
  app_clear(win, RGB(34, 32, 52));

  snprintf(prompt, 21, "%s > ", cwd);
  draw_string_fb(win->framebuff, win->w, win->h, prompt, 1, 1, NULL, NULL, RGB(200, 200, 200));
}
