#include "platform.h"

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

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
  
  memset(&platform->input, 0, sizeof platform->input);
  platform->mouse_wx = platform->mouse_wy = 0.0f;
  SDL_StartTextInput(platform->window);   // SDL3: typed text is opt-in

  return 1;
}


// update the platform (process events, update time)
void platform_update(struct platform *platform) {
  if (!platform) {
    LOG("platform_update: Platform pointer is NULL");
    return;
  }
 
  struct input *in = &platform->input;
  in->mouse_pressed = 0;
  in->mouse_released = 0;
  in->text[0] = '\0';
  in->key_count = 0;
 
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    switch (e.type) {
      case SDL_EVENT_QUIT:
        platform->running = false;
        break;
 
      case SDL_EVENT_KEY_DOWN: {
        if (e.key.key == SDLK_ESCAPE) {
          platform->running = false;
          break;
        }
 
        int k = NK_NONE;
        switch (e.key.key) {
          case SDLK_RETURN:    k = NK_ENTER;     break;
          case SDLK_BACKSPACE: k = NK_BACKSPACE; break;
          case SDLK_TAB:       k = NK_TAB;       break;
          case SDLK_UP:        k = NK_UP;        break;
          case SDLK_DOWN:      k = NK_DOWN;      break;
          case SDLK_LEFT:      k = NK_LEFT;      break;
          case SDLK_RIGHT:     k = NK_RIGHT;     break;
          case SDLK_HOME:      k = NK_HOME;      break;
          case SDLK_END:       k = NK_END;       break;
          case SDLK_DELETE:    k = NK_DELETE;    break;
          default: break;
        }
        if (k != NK_NONE && in->key_count < INPUT_KEYS_MAX) {
          in->keys[in->key_count++] = k;   // key repeat arrives as extra KEY_DOWN events
        }
        break;
      }
 
      case SDL_EVENT_TEXT_INPUT:
        SDL_strlcat(in->text, e.text.text, sizeof in->text);
        break;
 
      case SDL_EVENT_MOUSE_MOTION:
        platform->mouse_wx = e.motion.x;
        platform->mouse_wy = e.motion.y;
        break;
 
      case SDL_EVENT_MOUSE_BUTTON_DOWN:
        if (e.button.button == SDL_BUTTON_LEFT) {
          platform->mouse_wx = e.button.x;
          platform->mouse_wy = e.button.y;
          in->mouse_down = 1;
          in->mouse_pressed = 1;
        }
        break;
 
      case SDL_EVENT_MOUSE_BUTTON_UP:
        if (e.button.button == SDL_BUTTON_LEFT) {
          in->mouse_down = 0;
          in->mouse_released = 1;
        }
        break;
    }
  }
 
  // window pixel -> machine-screen pixel, undoing the CRT curvature
  SDL_GetWindowSizeInPixels(platform->window, &platform->win_w, &platform->win_h);
  float density = SDL_GetWindowPixelDensity(platform->window);
  in->mouse_valid = gfx_window_to_screen((int)(platform->mouse_wx * density),
                                         (int)(platform->mouse_wy * density),
                                         platform->win_w, platform->win_h,
                                         &in->mouse_x, &in->mouse_y);
 
  if (in->mouse_valid) SDL_HideCursor(); else SDL_ShowCursor();
  
  // ---- time / fps (unchanged from your version) ----
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


uint32_t *convert_bmp_to_framebuffer(const char *asset_name, int *width, int *height) {
  char fp[512];
  get_asset_path(fp, 512, asset_name);

  LOG("bitmap_load: %s", fp);

  FILE *file = fopen(fp, "rb");
  if (!file) {
    LOG("Error: Could not open BMP file %s\n", fp);
    return NULL;
  }

  // Read 54-byte header (BITMAPFILEHEADER + BITMAPINFOHEADER minimum)
  unsigned char header[54];
  if (fread(header, 1, sizeof(header), file) != sizeof(header)) {
    LOG("Error: BMP header too short: %s", fp);
    fclose(file);
    return NULL;
  }

  // Simple little-endian readers
  #define READ_LE32(p) ((uint32_t)(p)[0] | ((uint32_t)(p)[1] << 8) | ((uint32_t)(p)[2] << 16) | ((uint32_t)(p)[3] << 24))
  #define READ_LE16(p) ((uint16_t)(p)[0] | ((uint16_t)(p)[1] << 8))

  // Check BMP signature
  if (header[0] != 'B' || header[1] != 'M') {
    LOG("Error: Not a BMP file: %s", fp);
    fclose(file);
    return NULL;
  }

  uint32_t data_offset = READ_LE32(&header[10]);
  int32_t  w = (int32_t)READ_LE32(&header[18]);
  int32_t  h = (int32_t)READ_LE32(&header[22]);
  uint16_t planes = READ_LE16(&header[26]);
  uint16_t bit_count = READ_LE16(&header[28]);
  uint32_t compression = READ_LE32(&header[30]);

  if (planes != 1) {
    LOG("Error: Unsupported BMP planes=%u: %s", (unsigned)planes, fp);
    fclose(file);
    return NULL;
  }

  if (compression != 0) {
    LOG("Error: Compressed BMP not supported: %s", fp);
    fclose(file);
    return NULL;
  }

  int abs_h = h < 0 ? -h : h;
  int top_down = (h < 0);

  if (w <= 0 || abs_h <= 0) {
    LOG("Error: Invalid BMP dimensions %dx%d: %s", w, abs_h, fp);
    fclose(file);
    return NULL;
  }

  // Allocate framebuffer
  uint32_t *framebuffer = malloc((size_t)w * (size_t)abs_h * sizeof(uint32_t));
  if (!framebuffer) {
    LOG("Error: Could not allocate memory for framebuffer");
    fclose(file);
    return NULL;
  }

  // Seek to pixel data
  if (fseek(file, (long)data_offset, SEEK_SET) != 0) {
    LOG("Error: Failed to seek to pixel data: %s", fp);
    free(framebuffer);
    fclose(file);
    return NULL;
  }

  if (bit_count == 24) {
    int row_padding = (4 - (w * 3) % 4) % 4;
    for (int row = 0; row < abs_h; ++row) {
      int dst_row = top_down ? row : (abs_h - 1 - row);
      for (int x = 0; x < w; ++x) {
        unsigned char bgr[3];
        if (fread(bgr, 1, 3, file) != 3) {
          LOG("Error: Unexpected EOF while reading BMP pixels: %s\n");
          free(framebuffer);
          fclose(file);
          return NULL;
        }
        uint8_t r = bgr[2];
        uint8_t g = bgr[1];
        uint8_t b = bgr[0];
        framebuffer[dst_row * w + x] = RGB(r, g, b);
      }
      if (row_padding) fseek(file, row_padding, SEEK_CUR);
    }
  } else if (bit_count == 32) {
    // 32-bit BMP typically stores in BGRA order
    for (int row = 0; row < abs_h; ++row) {
      int dst_row = top_down ? row : (abs_h - 1 - row);
      for (int x = 0; x < w; ++x) {
        unsigned char bgra[4];
        if (fread(bgra, 1, 4, file) != 4) {
          LOG("Error: Unexpected EOF while reading BMP pixels: %s", fp);
          free(framebuffer);
          fclose(file);
          return NULL;
        }
        uint8_t r = bgra[2];
        uint8_t g = bgra[1];
        uint8_t b = bgra[0];
        framebuffer[dst_row * w + x] = RGB(r, g, b);
      }
    }
  } else {
    LOG("Error: Unsupported BMP bit depth %u: %s", (unsigned)bit_count, fp);
    free(framebuffer);
    fclose(file);
    return NULL;
  }

  fclose(file);
  *width = w;
  *height = abs_h;
  return framebuffer;
}
