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
  struct vfs_node_t *dir;
  
  if (parent == NULL) {
    dir = vfs_new_node("/", VFS_DIR, parent); 
  } else {
    dir = vfs_new_node(vfs_basename(real_path), VFS_DIR, parent);
  }

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


struct vfs_node_t *vfs_get_node(const struct vfs_node_t *fs_root, const char *path) {
  char *delim = "/";
  char *fp = calloc(strlen(path) + 1, sizeof(char));
  strcpy(fp, path);

  struct vfs_node_t *res = (struct vfs_node_t *)fs_root;
  char *token = strtok(fp, delim);

  while (token) {
    if (res->kind != VFS_DIR) {   // can't descend into a file
      res = NULL;
      break;
    }

    struct vfs_node_t *head = res->children;
    while (head && strcmp(head->name, token) != 0) {
      head = head->next;
    }

    if (!head) {
      res = NULL;
      break;
    }

    res = head;
    token = strtok(NULL, delim);
  }

  free(fp);
  return res;
}


// build node's path using tail recursion
static void vfs_get_node_path_recurse(const struct vfs_node_t *n, char *buf, size_t buf_len, unsigned level) {
  if (!n) return;

  vfs_get_node_path_recurse(n->parent, buf, buf_len, level + 1);

  strncat(buf, n->name, buf_len);
  if (strcmp(n->name, "/") != 0) { // don't append '/' to root dir because it's already named '/'
    if (n->kind == VFS_DIR || level != 0)
      strncat(buf, "/", buf_len);
  }
}


void vfs_get_node_path(const struct vfs_node_t *fs, char *buf, size_t buf_len) {
  vfs_get_node_path_recurse(fs, buf, buf_len, 0);
}


struct vfs_node_t *vfs_is_child(const struct vfs_node_t *root, char *name, unsigned trim_trailing_slash) {
  if (!root || !name || root->kind == VFS_FILE || !root->children) return NULL;

  if (trim_trailing_slash) {
    size_t name_len = strlen(name);
    
    // Check if the string is not empty and the last character is a slash
    if (name_len > 0 && name[name_len - 1] == '/') {
      name[name_len - 1] = '\0';
    }
  }

  struct vfs_node_t *head = root->children;
  while (head) {
    if (strcmp(head->name, name) == 0) return head;
    head = head->next;
  }

  return NULL;
}
