#include "apps.h"

#include <stddef.h>
#include <stdlib.h>

#include "test_pattern.h"

struct app_desc apps_registry[NUM_APPS] = {
  { "Trash", "icons/trash.bmp", 400, 320, 0, files_init, files_update, files_close },
  { "File", "icons/file_explorer.bmp", 400, 320, 0, files_init, files_update, files_close },
  { "Note", "icons/notes.bmp", 240, 320, 0, notes_init, notes_update, notes_close },
  { "Term", "icons/terminal.bmp", 400, 320, 0, NULL, terminal_update, NULL },
  { "Test", "icons/test_pattern.bmp", 200, 160, 0, NULL, draw_test_pattern, NULL },
};


void app_clear(struct window *win, uint32_t color) {
  if (!win) return;

  for (unsigned y = 0; y < win->h; ++y) {
    for (unsigned x = 0; x < win->w; ++x) {
      win->framebuff[y * win->w + x] = color;
    }
  }
}


void app_desc_free(struct app_desc *a) {
  if (a && a->icon_px) {
    free(a->icon_px);
  }
}