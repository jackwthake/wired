#ifndef __VFS_H__
#define __VFS_H__

#include <stddef.h>

#define MAX_FILE_NAME_LEN 64

enum vfs_kind { VFS_FILE, VFS_DIR };

struct vfs_node_t {
  char name[MAX_FILE_NAME_LEN];
  enum vfs_kind kind;

  char *content;               // NULL for directories
  size_t size;

  double mtime;                // game-time, not real time; can be set to lie
  unsigned hidden;             // for later "hidden files" mechanics

  struct vfs_node_t *parent;
  struct vfs_node_t *children; // linked list of children
  struct vfs_node_t *next;     // next sibling

  // optional: for files whose content is computed rather than static
  // (a resident-written log, a live process list)
  void (*generate)(struct vfs_node_t *self, char *out, size_t out_size);
};


struct vfs_node_t *vfs_load_dir(const char *real_path, struct vfs_node_t *parent);
void vfs_append(struct vfs_node_t *parent, struct vfs_node_t *child);

void vfs_free(struct vfs_node_t *n);


struct vfs_node_t *vfs_get_node_abs(const struct vfs_node_t *fs, char *path);
void vfs_get_node_path(const struct vfs_node_t *fs, char *buf, size_t buf_len);

#endif