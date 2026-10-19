#include "os_gfx.h"

// -------------------------------------------------------------------
// -- Keyboard -------------------------------------------------------
func B32
OS_KeyPressed(OS_KeyCode key) {
  if (_os_state.keyboard.current[key].pressed) {
    B32 result = !_os_state.keyboard.previous[key].pressed && _os_state.keyboard.current[key].pressed;
    return result;
  }
  return 0;
}

func B32
OS_KeyReleased(OS_KeyCode key) {
  if (_os_state.keyboard.previous[key].pressed) {
  B32 result = _os_state.keyboard.previous[key].pressed && !_os_state.keyboard.current[key].pressed;
  return result;
  }
  return 0;
}

func B32
OS_KeyDown(OS_KeyCode key) {
  B32 result = _os_state.keyboard.previous[key].pressed && _os_state.keyboard.current[key].pressed;
  return result;
}

func B32
OS_KeyUp(OS_KeyCode key) {
  B32 result = !_os_state.keyboard.previous[key].pressed && !_os_state.keyboard.current[key].pressed;
  return result;
}
