#include <navi/navi.h>
#include <gfx.h>
#include <platform.h>

#include <stdio.h>
#include <string.h>

static char *cwd = "/home/";

#define PROMPT_LENGTH 21
#define INPUT_LENGTH 512
#define HISTORY_LENGTH 1024

#define LINE_HEIGHT 16

static char history[HISTORY_LENGTH];
static char prompt[PROMPT_LENGTH];
static char input[INPUT_LENGTH];


static void process_line() {
  // update buffers
  char line[PROMPT_LENGTH + INPUT_LENGTH];

  if (history[0] == '\0') {
    snprintf(line, PROMPT_LENGTH + INPUT_LENGTH, "%s%s", prompt, input);
  } else {
    snprintf(line, PROMPT_LENGTH + INPUT_LENGTH, "\n%s%s", prompt, input);
  }

  strncat(history, line, HISTORY_LENGTH);

  // process line
  LOG("terminal: entered line: %s", input);

  // clear prompt and input for next frame
  prompt[0] = '\0';
  input[0] = '\0';
}


void terminal_update(struct window *win) {
  app_clear(win, RGB(34, 32, 52));

  snprintf(prompt, 21, "%s > ", cwd);
  strncat(input, win->in->text, INPUT_LENGTH);

  if (win->in->key_count) { // TODO: make this react to just return not all non alpha numeric keys
    LOG("line enterred");
    process_line();
  }

  unsigned next_start_x = 0;
  unsigned next_start_y = 1;

  draw_string_fb(win->framebuff, win->w, win->h, history, 1, 1, &next_start_x, &next_start_y, RGB(200, 200, 200));

  if (!*history) // only add line height offset if there is history to print
    draw_string_fb(win->framebuff, win->w, win->h, prompt, 1, next_start_y, &next_start_x, &next_start_y, RGB(200, 200, 200));
  else
    draw_string_fb(win->framebuff, win->w, win->h, prompt, 1, next_start_y + LINE_HEIGHT, &next_start_x, &next_start_y, RGB(200, 200, 200));

  draw_string_fb(win->framebuff, win->w, win->h, input, next_start_x, next_start_y, NULL, NULL, RGB(200, 200, 200));
}
