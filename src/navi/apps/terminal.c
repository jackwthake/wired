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


// ---------- Commands --------------------------------------------------------

#define CMD_STDOUT(prompt, str, is_cmd)                                     \
do {                                                                        \
  struct history_entry_t *res = create_history_entry(prompt, str, is_cmd);  \
  t->lines[t->num_lines++] = res;                                           \
} while (0)


typedef void(*cmd_run)(struct terminal_t *t, int argc, char **argv);

struct command {
  const char *name;
  cmd_run runner;
};


static void cmd_ls(struct terminal_t *t, int argc, char **argv) {
  if (argc > 2) {
    CMD_STDOUT(NULL, "USAGE: ls <directory>(optional)", 0);
    return;
  }

  struct vfs_node_t *n;
  
  if (argc == 2) {
    // TODO: make this accept multiple '../'s to go back multiple levels
    if (strcmp(argv[1], "..") == 0 || strcmp(argv[1], "../") == 0) {
      n = t->cwd->parent ? t->cwd->parent : t->cwd;
    } else {
      n = vfs_is_child(t->cwd, argv[1], true);
  
      if (!n) {
        CMD_STDOUT(NULL, "Directory does not exist.", 0);
        return;
      }
    }

    n = n->children;
  } else {
    n = t->cwd->children;
  }

  while (n) {
    if (n->kind == VFS_DIR) {
      char buf[FILENAME_MAX];
      snprintf(buf, FILENAME_MAX, "%s/", n->name);

      CMD_STDOUT(NULL, buf, 0);
    } else {
      CMD_STDOUT(NULL, n->name, 0);
    }

    n = n->next;
  }
}


static void cmd_cd(struct terminal_t *t, int argc, char **argv) {
  if (argc != 2) {
    CMD_STDOUT(NULL, "USAGE: cd <directory>", 0);
    return;
  }

  // TODO: make this accept multiple '../'s to go back multiple levels
  if (strcmp("..", argv[1]) == 0 || strcmp("../", argv[1]) == 0) {
    if (!t->cwd->parent) return;

    t->cwd = t->cwd->parent;
    return;
  }

  struct vfs_node_t *n = vfs_is_child(t->cwd, argv[1], true);

  if (!n || n->kind != VFS_DIR) {
    CMD_STDOUT(NULL, "Directory does not exist.", 0);
    return;
  }

  t->cwd = n;
}


static void cmd_cat(struct terminal_t *t, int argc, char **argv) {
  if (argc != 2) {
    CMD_STDOUT(NULL, "USAGE: cat <file>", 0);
    return;
  }

  struct vfs_node_t *n = vfs_is_child(t->cwd, argv[1], false);

  if (!n || n->kind != VFS_FILE) {
    CMD_STDOUT(NULL, "File does not exist or is a directory.", 0);
    return;
  }

  if (n->generate) {
    n->generate(n);
  }

  CMD_STDOUT(NULL, n->content, 0);
}


#define NUM_COMMANDS 3
static struct command commands[NUM_COMMANDS] = {
  { "ls",  cmd_ls },
  { "cd",  cmd_cd },
  { "cat", cmd_cat },
};


char** tokenize_input(char *str, size_t *out_count) {
  if (str == NULL || out_count == NULL) {
    if (out_count) *out_count = 0;
    return NULL;
  }
  
  size_t capacity = 8;
  size_t count = 0;
  char **tokens = malloc(capacity * sizeof(char*));
  if (!tokens) {
    *out_count = 0;
    return NULL;
  }
  
  char *token = strtok(str, " ");
  while (token != NULL) {
    if (count >= capacity) {
      capacity *= 2;
      char **new_tokens = realloc(tokens, capacity * sizeof(char*));
      if (!new_tokens) {
        free(tokens);
        *out_count = 0;
        return NULL;
      }
      tokens = new_tokens;
    }
    
    // Store pointer directly to the segment in str
    tokens[count++] = token;
    token = strtok(NULL, " ");
  }
  
  *out_count = count;
  return tokens;
}


static void process_line(struct terminal_t *t) {
  if (t->num_lines > t->max_lines) return; // TODO: wrap history;

  CMD_STDOUT(t->cwd_str, t->input, 1);

  if (strcmp(t->input, "") == 0) {
    goto skip_cmd;
  }

  char **argv;
  size_t argc;

  argv = tokenize_input(t->input, &argc);

  bool cmd_found = false;
  for (unsigned i = 0; i < NUM_COMMANDS; ++i) {
    if (strcmp(argv[0], commands[i].name) == 0) {
      commands[i].runner(t, argc, argv);

      if (argv)
        free(argv);

      cmd_found = true;
      break;
    }
  }

  if (!cmd_found) {
    CMD_STDOUT(NULL, "Unrecognized command.", 0);
  }

skip_cmd:  
  t->history_scroll = 0;
  t->input[0] = '\0';

  // update working directory string if the command changed it
  t->cwd_str[0] = '\0';
  vfs_get_node_path(t->cwd, t->cwd_str, PROMPT_LENGTH);
}


// ---------- Terminal Backend ------------------------------------------------

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


void terminal_init(struct window *win, struct vfs_node_t *path) {
  struct terminal_t *t = win->udata;

  t->lines = calloc(HISTORY_LENGTH, sizeof(struct terminal_t *));
  t->cwd_str = calloc(PROMPT_LENGTH, sizeof(char));
  t->input = calloc(INPUT_LENGTH, sizeof(char));

  t->num_lines = 0;
  t->max_lines = HISTORY_LENGTH;
  t->history_scroll = 0;
  
  t->cwd = path;
  vfs_get_node_path(t->cwd, t->cwd_str, PROMPT_LENGTH);
}


void terminal_update(struct window *win) {
  struct terminal_t *t = win->udata;
  const struct input *in = win->in;

  // Only the focused window receives keyboard input; background windows must not
  // touch the platform input struct.
  if (!t || !in) return;

  // grab text input
  strncat(t->input, in->text, INPUT_LENGTH);
  
  // check for return key
  for (int i = 0; i < in->key_count; ++i) {
    switch (in->keys[i]) {
      case NK_ENTER:
        if (strcmp(t->input, "exit") == 0) {
          win->close_requested = 1;
          return;
        }

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
  draw_string_fb(win->framebuff, win->w, win->h, t->cwd_str, 1, next_start_y, &next_start_x, &next_start_y, RGB(200, 200, 200));
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

    if (t->cwd_str) free(t->cwd_str);
    if (t->input)   free(t->input);

    t->num_lines = 0;
    t->max_lines = 0;
  }
}
