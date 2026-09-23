#include <navi/navi.h>
#include <gfx.h>
#include <platform.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "terminal.h"

#define PROMPT_LENGTH 21
#define INPUT_LENGTH 512
#define HISTORY_LENGTH 1024

#define LINE_HEIGHT 16

static const char *prompt_sep = " $ ";

static struct history_entry_t *create_history_entry(char *prompt, char *line, unsigned is_cmd) {
  struct history_entry_t *e = calloc(1, sizeof(struct history_entry_t));

  e->is_command = is_cmd;
  e->line_length = strlen(line);
  
  if (prompt) {
    e->prompt_length = strlen(prompt);
    
    e->prompt = calloc(e->prompt_length + 1, sizeof(char));
    strcpy(e->prompt, prompt);
  }

  e->line = calloc(e->line_length + 1, sizeof(char));
  strcpy(e->line, line);

  return e;
}


static void free_history_entry(struct history_entry_t *e) {
  if (!e) return;

  if (e->prompt) free(e->prompt);
  if (e->line) free(e->line);

  e->prompt_length = 0;
  e->line_length = 0;
  free(e);
}


static void process_line(struct terminal_t *t) {
  if (t->num_lines > t->max_lines) return; // TODO: wrap history;

  struct history_entry_t *e = create_history_entry(t->cwd, t->input, 1);
  t->lines[t->num_lines++] = e;

  // temporary echo command
  struct history_entry_t *res = create_history_entry(NULL, t->input, 0);
  t->lines[t->num_lines++] = res;

  t->history_scroll = 0;
  t->input[0] = '\0';
}


static void get_prev(struct terminal_t *t) {
  if (!t || !t->lines || t->num_lines == 0) return;

  while (t->history_scroll < t->num_lines) {
    unsigned idx = t->num_lines - (++t->history_scroll);

    if (idx >= t->num_lines) {
      t->history_scroll = t->num_lines;
      return;
    }

    struct history_entry_t *entry = t->lines[idx];
    if (!entry) continue;

    if (entry->is_command) {
      strcpy(t->input, entry->line);
      return;
    }
  }
}


static void get_next(struct terminal_t *t) {
  if (!t || !t->lines || t->num_lines == 0) return;

  while (t->history_scroll > 0) {
    --t->history_scroll;

    unsigned idx = t->num_lines - t->history_scroll;
    if (idx >= t->num_lines) {
      t->history_scroll = 0;
      t->input[0] = '\0';
      return;
    }

    struct history_entry_t *entry = t->lines[idx];
    if (entry && entry->is_command) {
      strcpy(t->input, entry->line);
      return;
    }
  }

  t->input[0] = '\0';
  t->history_scroll = 0;
}


void terminal_init(struct window *win) {
  struct terminal_t *t = win->udata;

  t->lines = calloc(HISTORY_LENGTH, sizeof(struct terminal_t *));
  t->cwd = calloc(PROMPT_LENGTH, sizeof(char));
  t->input = calloc(INPUT_LENGTH, sizeof(char));

  t->num_lines = 0;
  t->max_lines = HISTORY_LENGTH;
  t->history_scroll = 0;
  
  strcpy(t->cwd, "/home/");
}


void terminal_update(struct window *win) {
  struct terminal_t *t = win->udata;
  const struct input *in = win->in;

  // Only the focused window receives keyboard input; background windows must not
  // touch the platform input struct.
  if (!t || !in) return;

  // grab text input
  strcat(t->input, in->text);
  
  // check for return key
  for (int i = 0; i < in->key_count; ++i) {
    switch (in->keys[i]) {
      case NK_ENTER:
        process_line(t);
        break;
      
      case NK_BACKSPACE:
        unsigned len = strlen(t->input);
        if (len > 0) t->input[len - 1] = '\0';
        break;
      
      case NK_UP:
        get_prev(t);
        break;
      
      case NK_DOWN:
        get_next(t);
        break;

      default: break;
    }
  }

  // render
  app_clear(win, RGB(34, 32, 52));

  unsigned next_start_x = 1;
  unsigned next_start_y = 1;

  // draw history
  for (unsigned i = t->display_start; i < t->num_lines; ++i) {
    if (t->lines[i]->is_command) {
      draw_string_fb(win->framebuff, win->w, win->h, t->lines[i]->prompt, 1, next_start_y, &next_start_x, &next_start_y, RGB(200, 200, 200));
      draw_string_fb(win->framebuff, win->w, win->h, prompt_sep, next_start_x, next_start_y, &next_start_x, &next_start_y, RGB(200, 200, 200));
      draw_string_fb(win->framebuff, win->w, win->h, t->lines[i]->line, next_start_x, next_start_y, &next_start_x, &next_start_y, RGB(200, 200, 200));
    } else {
      draw_string_fb(win->framebuff, win->w, win->h, t->lines[i]->line, 1, next_start_y, &next_start_x, &next_start_y, RGB(200, 200, 200));
    }

    next_start_y += LINE_HEIGHT;
  }

  // draw prompt
  draw_string_fb(win->framebuff, win->w, win->h, t->cwd, 1, next_start_y, &next_start_x, &next_start_y, RGB(200, 200, 200));
  draw_string_fb(win->framebuff, win->w, win->h, prompt_sep, next_start_x, next_start_y, &next_start_x, &next_start_y, RGB(200, 200, 200));
  draw_string_fb(win->framebuff, win->w, win->h, t->input, next_start_x, next_start_y, &next_start_x, &next_start_y, RGB(200, 200, 200));

  // draw cursor
  // blink: on for one half-second, off for the next
  double period = 0.5;

  if (fmod(in->time, period * 2.0) < period) {
    draw_rect(win->framebuff, win->w, win->h, next_start_x + 1, next_start_y + (LINE_HEIGHT * 0.6f), 8, 4, RGB(200, 200, 200));
  }
  
  // this is imperfect because a line entry could have multiple entries but this is fine for now.
  if (next_start_y + LINE_HEIGHT > win->h) {
    ++t->display_start;
  }
}


void terminal_close(struct window *win) {
  struct terminal_t *t = win->udata;
  
  if (t) {
    if (t->lines) {
      for (unsigned i = 0; i < t->num_lines; ++i) {
        free_history_entry(t->lines[i]);
      }

      free(t->lines);
    }

    if (t->cwd)   free(t->cwd);
    if (t->input) free(t->input);

    t->num_lines = 0;
    t->max_lines = 0;
  }
}
