#include "platform.h"


int main(int argc, char* argv[]) {
  struct platform platform;

  if (!platform_init(&platform)) {
    return 1;
  }

  while (platform.running) {
    platform_update(&platform);

    platform_clear(&platform, 0.02f, 0.03f, 0.08f, 1.0f);
    platform_swap_buffers(&platform);

    // sleep for a short duration to avoid busy waiting
    SDL_Delay(16); // ~60 FPS
  }

  platform_shutdown(&platform);
  return 0;
}
