#include "vfs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

#include <platform.h>

static struct vfs_node_t *vfs_new_node(char *name, enum vfs_kind kind, struct vfs_node_t *parent) {
  struct vfs_node_t *n = calloc(1, sizeof(struct vfs_node_t));

  unsigned name_len = strlen(name) + 1;
  name_len = name_len > MAX_FILE_NAME_LEN ? MAX_FILE_NAME_LEN : name_len;

  strncpy(n->name, name, name_len);
  n->name[name_len] = '\0';

  n->kind = kind;
  n->parent = parent;

  return n;
}


static char *vfs_basename(const char *path) {
  if (!path || !*path) return (char*)path;

  char *end = (char*)path + strlen(path);
  while (end > path && end[-1] == '/') {
    end--;
  }

  if (end == path) return (char*)path;

  char *start = end;
  while (start > path && start[-1] != '/') {
    start--;
  }

  return start;
}


void vfs_free(struct vfs_node_t *n) {
  if (!n) return;

  if (n->children) {
    struct vfs_node_t *c = n->children;
    while (c) {
      struct vfs_node_t *tmp = n->next;
      
      vfs_free(c);
      c = tmp;
    }
  }

  free(n);
}


void vfs_append(struct vfs_node_t *parent, struct vfs_node_t *n) {
  if (!parent) return;

  struct vfs_node_t *tmp = parent->children;
  parent->children = n;
  n->next = tmp;
}


struct vfs_node_t *vfs_load_dir(const char *real_path, struct vfs_node_t *parent) {
  struct vfs_node_t *dir = vfs_new_node(vfs_basename(real_path), VFS_DIR, parent);

  DIR *d = opendir(real_path);
  if (!d) return dir;

  struct dirent *e;
  while ((e = readdir(d))) {
    if (e->d_name[0] == '.') continue;   // skip ./..

    char child_path[512];
    snprintf(child_path, sizeof child_path, "%s/%s", real_path, e->d_name);

    struct stat st;
    stat(child_path, &st);

    if (S_ISDIR(st.st_mode)) {
      vfs_append(dir, vfs_load_dir(child_path, dir));
    } else {
      struct vfs_node_t *f = vfs_new_node(e->d_name, VFS_FILE, dir);

      f->content = load_asset_file_abs(child_path);
      f->size = strlen(f->content);
      f->mtime = (double)st.st_mtime;
      vfs_append(dir, f);
    }
  }

  closedir(d);
  return dir;
}