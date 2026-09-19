#include "platform.h"


const char *const WINDOW_TITLE = "Wired";
const unsigned int const SCREEN_WIDTH = 800;
const unsigned int const SCREEN_HEIGHT = 600;


// Platform functions

// initialize the platform (SDL, window, OpenGL context)
unsigned platform_init(struct platform *platform) {
  if (!platform) {
    LOG("platform_init: Platform pointer is NULL");
    return 0;
  }

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    LOG("platform_init: SDL_Init failed: %s", SDL_GetError());
    return 0;
  }
  
  // Must be set BEFORE creating the window
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
  
  platform->window = SDL_CreateWindow(WINDOW_TITLE, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
  if (!platform->window) {
    LOG("platform_init: Window creation failed: %s", SDL_GetError());
    return 0;
  }
  
  platform->gl = SDL_GL_CreateContext(platform->window);

  if (!platform->gl) {
    LOG("platform_init: GL context failed: %s", SDL_GetError());
    return 0;
  }
  
  SDL_GL_SetSwapInterval(1); // vsync

  platform->running = 1;
  platform->platform_time = SDL_GetTicksNS() / 1e9;
  platform->platform_time_delta = 0.0;
  platform->frames = 0;
  platform->fps_time_accum = 0.0;
  platform->fps = 0;

  LOG("platform_init: Video driver: %s", SDL_GetCurrentVideoDriver());
  LOG("platform_init: GL_VERSION: %s", (const char*)glGetString(GL_VERSION));
  
  return 1;
}


// update the platform (process events, update time)
void platform_update(struct platform *platform) {
  if (!platform) {
    LOG("platform_update: Platform pointer is NULL");
    return;
  }

  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    switch (e.type) {
      case SDL_EVENT_QUIT:
        platform->running = false;
        break;

      case SDL_EVENT_KEY_DOWN:
        if (e.key.key == SDLK_ESCAPE) {
          platform->running = false;
        }
        break;
    }
  }

  Uint64 now_ns = SDL_GetTicksNS();
  double now = now_ns / 1e9;

  platform->platform_time_delta = now - platform->platform_time;
  platform->platform_time = now;

  if (platform->platform_time_delta > 0.1) {
    LOG("platform_update: Warning: Time delta is too large, possible frame drop or pause.");
  }

  platform->fps_time_accum += platform->platform_time_delta;
  if (platform->fps_time_accum >= 1.0) {
    platform->fps = platform->frames;
    platform->frames = 0;
    platform->fps_time_accum = 0.0;
    LOG("platform_update: FPS: %u, Time Delta: %.6f seconds", platform->fps, platform->platform_time_delta);
  }
}


// clear the screen with a color
void platform_clear(struct platform *platform, float r, float g, float b, float a) {
  if (!platform) {
    LOG("platform_clear: Platform pointer is NULL");
    return;
  }

  glClearColor(r, g, b, a);
  glClear(GL_COLOR_BUFFER_BIT);
}


void platform_swap_buffers(struct platform *platform) {
  if (!platform) {
    LOG("platform_swap_buffers: Platform pointer is NULL");
    return;
  }

  SDL_GL_SwapWindow(platform->window);
  ++platform->frames;
}


void platform_shutdown(struct platform *platform) {
  if (!platform) {
    LOG("platform_shutdown: Platform pointer is NULL");
    return;
  }

  SDL_GL_DestroyContext(platform->gl);
  SDL_DestroyWindow(platform->window);
  SDL_Quit();
}