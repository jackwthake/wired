#include <SDL3/SDL.h>
#include <SDL3/SDL_opengles2.h>

int main(int argc, char* argv[]) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("SDL_Init failed: %s", SDL_GetError());
    return 1;
  }
  
  // Must be set BEFORE creating the window
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
  
  SDL_Window* window = SDL_CreateWindow("Wired", 800, 600, SDL_WINDOW_OPENGL);
  if (!window) {
    SDL_Log("Window creation failed: %s", SDL_GetError());
    return 1;
  }
  
  SDL_GLContext gl = SDL_GL_CreateContext(window);
  if (!gl) {
    SDL_Log("GL context failed: %s", SDL_GetError());
    return 1;
  }
  SDL_GL_SetSwapInterval(1); // vsync
  
  SDL_Log("Video driver: %s", SDL_GetCurrentVideoDriver());
  SDL_Log("GL_VERSION: %s", (const char*)glGetString(GL_VERSION));
  
  bool running = true;
  while (running) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_EVENT_QUIT) running = false;
    }
    
    glClearColor(0.02f, 0.03f, 0.08f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    SDL_GL_SwapWindow(window); // this is what actually maps the window on Wayland
  }
  
  SDL_GL_DestroyContext(window ? gl : NULL);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}