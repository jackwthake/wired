#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengles2.h>

#include "scenes/input.h"


#ifndef LOG
#ifdef DEBUG
  #define LOG(...) SDL_Log(__VA_ARGS__)
#else
  #define LOG(...) ((void)0)
#endif
#endif


struct platform {
  SDL_Window *window;
  SDL_GLContext gl;

  unsigned running;

  double platform_time;
  double platform_time_delta;

  unsigned frames;
  double fps_time_accum;
  unsigned fps;

  struct input input;
  float mouse_wx, mouse_wy;   // last mouse position in window coordinates
  int win_w, win_h;           // current window size in pixels (pass to gfx_present)
};


unsigned platform_init(struct platform *platform);

void platform_update(struct platform *platform);
void platform_swap_buffers(struct platform *platform);

void platform_shutdown(struct platform *platform);


unsigned get_asset_path(char *buffer, size_t buffer_size, const char *asset_name);
uint32_t *convert_bmp_to_framebuffer(const char *asset_name, int *width, int *height);

#endif // __PLATFORM_H__