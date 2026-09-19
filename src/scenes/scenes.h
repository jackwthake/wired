#ifndef __SCENES_H__
#define __SCENES_H__

#include <stddef.h>

#include "fsm.h"

enum states {
  BOOT_STATE = 0,
  DESKTOP_STATE,
  NUM_STATES
};

extern state_interface_t boot_scene;
extern state_interface_t desktop_scene; 

void boot_enter(void *n, size_t s);
void boot_tick(void *n, size_t s, float dt);
int boot_render(void *n, size_t s);

void desktop_enter(void *n, size_t s);
void desktop_tick(void *n, size_t s, float dt);
int desktop_render(void *n, size_t s);

#endif