#include "apps.h"

#include <stddef.h>
#include <stdlib.h>

#include "test_pattern.h"

struct app_desc apps_registry[NUM_APPS] = {
  { "Test", "icons/test_pattern.bmp", 200, 160, 0, NULL, draw_test_pattern, NULL },
  { "Test", "icons/test_pattern.bmp", 200, 160, 0, NULL, draw_test_pattern, NULL },
  { "Test", "icons/test_pattern.bmp", 200, 160, 0, NULL, draw_test_pattern, NULL },
  { "Test", "icons/test_pattern.bmp", 200, 160, 0, NULL, draw_test_pattern, NULL },
  { "Test", "icons/test_pattern.bmp", 200, 160, 0, NULL, draw_test_pattern, NULL },
  { "Test", "icons/test_pattern.bmp", 200, 160, 0, NULL, draw_test_pattern, NULL },
  { "Test", "icons/test_pattern.bmp", 200, 160, 0, NULL, draw_test_pattern, NULL },
  { "Test", "icons/test_pattern.bmp", 200, 160, 0, NULL, draw_test_pattern, NULL },
  { "Test", "icons/test_pattern.bmp", 200, 160, 0, NULL, draw_test_pattern, NULL },
  { "Test", "icons/test_pattern.bmp", 200, 160, 0, NULL, draw_test_pattern, NULL },
  { "Test", "icons/test_pattern.bmp", 200, 160, 0, NULL, draw_test_pattern, NULL },
  { "Test", "icons/test_pattern.bmp", 200, 160, 0, NULL, draw_test_pattern, NULL },
  { "Test", "icons/test_pattern.bmp", 200, 160, 0, NULL, draw_test_pattern, NULL },
  { "Test", "icons/test_pattern.bmp", 200, 160, 0, NULL, draw_test_pattern, NULL },
  { "Test", "icons/test_pattern.bmp", 200, 160, 0, NULL, draw_test_pattern, NULL },
  { "Test", "icons/test_pattern.bmp", 200, 160, 0, NULL, draw_test_pattern, NULL },
  { "Test", "icons/test_pattern.bmp", 200, 160, 0, NULL, draw_test_pattern, NULL },
  { "Test", "icons/test_pattern.bmp", 200, 160, 0, NULL, draw_test_pattern, NULL },
  { "Test", "icons/test_pattern.bmp", 200, 160, 0, NULL, draw_test_pattern, NULL },
  { "Test", "icons/test_pattern.bmp", 200, 160, 0, NULL, draw_test_pattern, NULL },
  { "Test", "icons/test_pattern.bmp", 200, 160, 0, NULL, draw_test_pattern, NULL },
};

void app_desc_free(struct app_desc *a) {
  if (a && a->icon_px) {
    free(a->icon_px);
  }
}