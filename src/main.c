#include "platform.h"

#include "gfx.h"
#include "fsm.h"

#include "scenes/scenes.h"

extern const unsigned int PLATFORM_SCREEN_WIDTH;
extern const unsigned int PLATFORM_SCREEN_HEIGHT;

state_machine_t main_state;


int main(int argc, char* argv[]) {
  struct platform platform;

  if (!platform_init(&platform)) {
    return 1;
  }

  if (!gfx_init()) {
    LOG("main: gfx_init failed");
    platform_shutdown(&platform);
    return 1;
  }

  fsm_init(&main_state, BOOT_STATE, NUM_STATES);
  fsm_set_state_interface(&main_state, BOOT_STATE, &boot_scene);
  fsm_set_state_interface(&main_state, DESKTOP_STATE, &desktop_scene);

  if (!fsm_start(&main_state)) {
    LOG("main: fsm init failed");
    platform_shutdown(&platform);
    return 1;
  }

  while (platform.running) {
    platform_update(&platform);
    fsm_tick_state(&main_state, platform.platform_time_delta);

    fsm_render_state(&main_state);

    gfx_upload();    
    gfx_present(PLATFORM_SCREEN_WIDTH, PLATFORM_SCREEN_HEIGHT, platform.platform_time);
    
    platform_swap_buffers(&platform);

    // sleep for a short duration to avoid busy waiting
    SDL_Delay(16); // ~60 FPS
  }

  fsm_free(&main_state);
  platform_shutdown(&platform);
  return 0;
}
