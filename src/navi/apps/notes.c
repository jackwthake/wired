#include "notes.h"

#include <navi/navi.h>
#include <gfx.h>

#include <stdio.h>

void notes_init(struct window *win, struct vfs_node_t *path) {
  struct notes_t *notes = win->udata;

  if (!path) {
    win->close_requested = 1;
  }

  if (path->kind == VFS_FILE) {
    notes->content = path->content;     // we dont manage this memmory
    notes->content_size = path->size;
    notes->file = path;
  } else {
    notes->content = NULL;
    notes->content_size = 0;
    notes->file = NULL;
  }
}


void notes_update(struct window *win) {
  struct notes_t *notes = win->udata;

  app_clear(win, RGB(200, 200, 200));

  if (notes->content_size) {
    int next_x = 1, next_y = 1;

    text_wrap_region_draw_string(win->framebuff, win->w, win->h, 80, notes->content, 1, 1, &next_x, &next_y, RGB(10, 10, 10));
  }
}


void notes_close(struct window *win) {
  
}
