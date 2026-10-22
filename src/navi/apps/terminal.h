#ifndef __TERMINAL_H__
#define __TERMINAL_H__

#include <vfs.h>
#include <navi/navi.h>

#define TERMINAL_MAX_COLS 60

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
  struct navi_scroll_region *scroll_region;
};

#endif