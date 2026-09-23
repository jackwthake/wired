#ifndef __TERMINAL_H__
#define __TERMINAL_H__

struct history_entry_t {
  char *prompt;
  char *line;

  unsigned is_command;
  unsigned int prompt_length;
  unsigned int line_length;
};

struct terminal_t {
  struct history_entry_t **lines;
  char *input;
  char *cwd;

  int history_scroll;
  unsigned int max_lines;
  unsigned int num_lines;

  // scrolling
  unsigned display_start;
};

#endif