#ifndef __NAVI_H__
#define __NAVI_H__

#include <stdint.h>

struct window;
typedef void (*navi_win_callback)(struct window *);

struct window {
  unsigned x, y, w, h;
  char *title;
  
  uint32_t *framebuff;
  
  void *udata;
  unsigned udata_size;
  
  navi_win_callback update;
};


struct navi_t {
  struct window *open_windows;
  unsigned max_windows, next_idx;
};


void navi_init(struct navi_t *navi, unsigned max_open);
struct window *navi_add_window(struct navi_t *navi, unsigned x, unsigned y, unsigned w, unsigned h, 
                         char *title, void *udata, unsigned udata_size, navi_win_callback updatefn);

void navi_update_windows(struct navi_t *navi);


void draw_bitmap_to_framebuffer(uint32_t *framebuffer, int fb_w, int fb_h, uint32_t *bitmap, int bmp_w, int bmp_h, int x_offset, int y_offset);
void draw_string_to_framebuffer(uint32_t *framebuffer, const char *str, int x_offset, int y_offset, int *next_char_x, int *next_char_y, uint32_t color);

#endif // __NAVI_H__