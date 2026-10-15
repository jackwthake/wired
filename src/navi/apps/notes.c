#include "notes.h"

#include <platform.h>
#include <navi/navi.h>
#include <gfx.h>

#include <string.h>
#include <stdlib.h>
#include <math.h>

#define BUFFER_BUMP_AMOUNT 128

void notes_init(struct window *win, struct vfs_node_t *path) {
  struct notes_t *notes = win->udata;
  notes->cursor_pos = 0;

  if (!path) {
    win->close_requested = 1;
  }

  if (path->kind == VFS_FILE) {
    notes->content = strndup(path->content, path->size);
    notes->content_size = path->size;
    notes->file = path;
  } else {
    notes->content = calloc(BUFFER_BUMP_AMOUNT, sizeof(char));
    notes->content_size = BUFFER_BUMP_AMOUNT;
    notes->file = NULL;
  }
}


static void bump_notes_buffer(struct notes_t *notes, size_t desired_size) {
  if (desired_size > notes->content_size) {
    size_t new_size = notes->content_size + BUFFER_BUMP_AMOUNT;

    notes->content = realloc(notes->content, new_size);
    notes->content_size = new_size;
  }
}


void notes_update(struct window *win) {
  struct notes_t *notes = win->udata;

  // Only the focused window receives keyboard input; background windows must not
  // touch the platform input struct.
  if (!notes || !win->in) return;

  for (int i = 0; i < win->in->key_count; ++i) {
    switch (win->in->keys[i]) {
      case NK_ENTER:
        bump_notes_buffer(notes, notes->content_size + 2);
        strcat(notes->content, "\n");
        break;
      case NK_TAB:
        bump_notes_buffer(notes, notes->content_size + 2);
        strcat(notes->content, "\t");
        break;
      case CTRL_S:
        if (notes->file) {
          free(notes->file->content);
          notes->file->content = strndup(notes->content, strlen(notes->content));
          notes->file->size = strlen(notes->content);
          notes->file->mtime = win->in->time;
        }
      case NK_BACKSPACE:
        if (strlen(notes->content) > 0) {
          notes->content[strlen(notes->content) - 1] = '\0';
        }
        break;
      default:
        break;
    }
  }
  
  if (win->in && win->in->text[0] != '\0') {
    size_t text_size = notes->content_size + strlen(win->in->text) + 1;
    bump_notes_buffer(notes, text_size);
    strcat(notes->content, win->in->text);
  }

  app_clear(win, RGB(200, 200, 200));

  int next_x = 1, next_y = 1;

  if (notes->content_size) {
    text_wrap_region_draw_string(win->framebuff, win->w, win->h, NOTES_MAX_COLS, notes->content, 1, 1, &next_x, &next_y, RGB(10, 10, 10));
  }

  long long current_block = (long long)(win->in->time / 0.5);
  int show_cursor = (current_block % 2) == 0;

  if (show_cursor) {
    draw_rect(win->framebuff, win->w, win->h, next_x, next_y, GLYPH_WIDTH, GLYPH_HEIGHT, RGB(10, 10, 10));
  }
}


void notes_close(struct window *win) {
  struct notes_t *notes = win->udata;

  free(notes->content);
}
