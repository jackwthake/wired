#ifndef __NOTES_H__
#define __NOTES_H__

#include <vfs.h>

#define NOTES_MAX_COLS 60

struct notes_t {
  struct vfs_node_t *file;
  
  unsigned cursor_pos;
  char *content;
  unsigned content_size;
};

#endif // __NOTES_H__