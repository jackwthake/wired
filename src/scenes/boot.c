#include "scenes.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../gfx.h"
#include "../fsm.h"
#include "navi.h"

extern state_machine_t main_state;

static const char *boot_sequence[] = {
  "NEURALINK BIOS v4.71.2 (c) 2079 Kessler-Vance Systems",
  "CPU: 8x QUANTA-9 @ 4.2THz ............................ OK",
  "MEM: 65536 PB HOLOGRAPHIC CORE ....................... OK",
  "Probing bus 0x7F... 14 devices found",
  "Loading kernel: KVOS_GHOST.sys",
  "Mounting /dev/synapse0 as root",
  "WARNING: unsigned firmware detected on /dev/optic2",
  "Bypassing corporate DRM handshake ............ DONE",
  "Initializing ICE shield, layer 1 of 7",
  "Neural handshake: awaiting operator...",
  "Handshake accepted. Latency 3ms",
  "Starting netd on 0.0.0.0:31337",
  "Spoofing MAC address ......................... OK",
  "Routing through 6 anonymous relays",
  "Loading daemon: shadowwatch (PID 1138)",
  "ALERT: tracer ping from sector 9, ignoring",
  "Decrypting user profile .................... OK",
  "No matter where you are, we are all connected."
};

static unsigned lines_revealed = 1;
static unsigned max_lines = 18;

void boot_enter(void *n, size_t s) {
  srand(time(NULL));
}


void boot_tick(void *n, size_t s, float dt) {
  if (rand() % 100 <=2) ++lines_revealed;

  if (lines_revealed > max_lines) {
    fsm_change_state(&main_state, DESKTOP_STATE);
  }
}


int boot_render(void *n, size_t s) {
  uint32_t *pixels = gfx_pixels();
  char revealed[1024];

  revealed[0] = '\0';
  size_t rem = sizeof revealed;
  size_t off = 0;
  unsigned seq_len = sizeof boot_sequence / sizeof boot_sequence[0];
  unsigned show = lines_revealed;
  if (show > seq_len) show = seq_len;

  for (unsigned i = 0; i < show; ++i) {
    int n = snprintf(revealed + off, rem, "%s\n", boot_sequence[i]);
    if (n < 0) break;
    if ((size_t)n >= rem) { /* truncated */
      off += rem - 1;
      rem = 0;
      break;
    }
    off += (size_t)n;
    rem -= (size_t)n;
  }

  draw_string_to_framebuffer(pixels, revealed, 10, 10, NULL, NULL, RGB(200, 200, 200));
  return 0;
}


state_interface_t boot_scene = {
  boot_enter,
  boot_tick,
  boot_render,
  NULL
};
