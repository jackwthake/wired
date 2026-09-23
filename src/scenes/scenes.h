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

#endif