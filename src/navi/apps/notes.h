#ifndef __NOTES_H__
#define __NOTES_H__

#include <vfs.h>

struct notes_t {
  struct vfs_node_t *file;
  
  char *content;
  unsigned content_size;
};

#endif // __NOTES_H__