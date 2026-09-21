#include "scenes.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "gfx.h"
#include "fsm.h"
#include "navi/navi.h"

extern state_machine_t main_state;

static const char *boot_sequence[] = {
  "NAVI/OS 0.9.3",
  "CPU: 8x QUANTA-9 @ 4.2THz ..................... OK",
  "MEM: 65536 PB HOLOGRAPHIC CORE ................ OK",
  "probing bus ......... 3 devices",
  "  hd0    ok",
  "  nic0   no carrier",
  "  ???0   responding (no driver)",
  "mounting /              ok (unclean shutdown)",
  "mounting /home          ok"
  "mounting /dev/synapse0  ok",
  "fsck: session table: 1 entry has no end",
  "WARNING: unsigned firmware detected on /dev/optic2",
  "bypassing corporate DRM handshake ............ DONE",
  "loading /etc/navi/continuity.conf",
  "clock: skew +212d (source: unknown)",
  "Starting netd on 0.0.0.0:31337",
  "Spoofing MAC address ......................... OK",
  "deletion job 0031: resuming (age 2209d)",
  "decrypting user profile ...................... OK",
  "resident: attached",
  "all systems nominal."
  ""
};

static unsigned lines_revealed = 0;
static unsigned max_lines = 22;

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
