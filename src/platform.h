#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengles2.h>


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
};


unsigned platform_init(struct platform *platform);

void platform_update(struct platform *platform);

void platform_clear(struct platform *platform, float r, float g, float b, float a);
void platform_swap_buffers(struct platform *platform);

void platform_shutdown(struct platform *platform);

#endif // __PLATFORM_H__