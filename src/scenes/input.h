#ifndef __INPUT_H__
#define __INPUT_H__

#define INPUT_TEXT_MAX 64
#define INPUT_KEYS_MAX 16

// Special keys the desktop cares about. Printable characters arrive as UTF-8
// text instead, so keyboard layouts and IME are handled by the platform layer.
enum navi_key {
  NK_NONE = 0,
  NK_ENTER, NK_BACKSPACE, NK_TAB,
  NK_UP, NK_DOWN, NK_LEFT, NK_RIGHT,
  NK_HOME, NK_END, NK_DELETE
};

// One frame of input. Filled by the platform layer, read by navi.
struct input {
  int mouse_x, mouse_y;   // machine-screen pixels (CRT curvature already undone)
  int mouse_valid;        // 0 when the cursor is outside the curved screen
  int mouse_down;         // left button held
  int mouse_pressed;      // left button went down this frame
  int mouse_released;     // left button came up this frame

  char text[INPUT_TEXT_MAX];   // UTF-8 typed this frame ("" if none)
  int  keys[INPUT_KEYS_MAX];   // enum navi_key presses this frame
  int  key_count;
};

#endif // __INPUT_H__