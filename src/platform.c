#include "platform.h"

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#include "gfx.h"

const char *const WINDOW_TITLE = "Wired";
const unsigned int PLATFORM_SCREEN_WIDTH = SCREEN_W * 2; // scale up the internal resolution for the window;
const unsigned int PLATFORM_SCREEN_HEIGHT = SCREEN_H * 2; // scale up the internal resolution for the window;


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
  
  platform->window = SDL_CreateWindow(WINDOW_TITLE, PLATFORM_SCREEN_WIDTH, PLATFORM_SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
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


// resolve path to executable and append "assets/" to it, then append the asset_name
unsigned get_asset_path(char *buffer, size_t buffer_size, const char *asset_name) {
  char exe[512];
  char try_path[512];

  ssize_t len = readlink("/proc/self/exe", exe, sizeof(exe) - 1);
  if (len > 0) {
    exe[len] = '\0';
    char *last = strrchr(exe, '/');

    if (last) {
      *last = '\0'; // trim to directory
      snprintf(try_path, sizeof(try_path), "%s/assets/%s", exe, asset_name);

      if (access(try_path, R_OK) == 0) {
        strncpy(buffer, try_path, buffer_size);
        buffer[buffer_size-1] = '\0';

        return 1;
      }
    }
  }

  return 0;
}