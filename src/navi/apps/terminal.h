#ifndef __TERMINAL_H__
#define __TERMINAL_H__

#include <vfs.h>

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
  
  struct vfs_node_t *cwd;
  char *cwd_str;

  int history_scroll;
  unsigned int max_lines;
  unsigned int num_lines;

  // scrolling
  unsigned display_start;
};

#endif