#include "os_linux_wayland_gfx.h"

#include <sys/mman.h>
#include <unistd.h>
#include <time.h>

func void
OS_Init(U64 arena_size)
{
  _os_state.arena = AllocateArena(arena_size);
}

func void
_ShellHandlePing(void* data, xdg_wm_base* shell, U32 serial)
{
  OS_WindowHandle* handle = (OS_WindowHandle*)data;

  xdg_wm_base_pong(handle->shell, serial);
}

func void
_PointerHandleEnter(void* data, wl_pointer* pointer, U32 serial, wl_surface* surface, I32 surface_x, I32 surface_y)
{
  OS_WindowHandle* handle = (OS_WindowHandle*)data;

  handle->pointer_enter_serial = serial;

  wl_pointer_set_cursor(handle->pointer, handle->pointer_enter_serial, 0, 0, 0);

  OS_Event event = {
    .type = OS_EVENT_TYPE_MOUSE_ENTER,
  };
  OS_EventListPush(&_os_state.event_list, event);
}

func void
_PointerHandleLeave(void*data, wl_pointer* pointer, U32 serial, wl_surface* surface)
{
  OS_Event event = {
    .type = OS_EVENT_TYPE_MOUSE_LEAVE,
  };
  OS_EventListPush(&_os_state.event_list, event);
}

func void
_PointerHandleMotion(void* data, wl_pointer* pointer, U32 time, I32 surface_x, I32 surface_y)
{
  OS_WindowHandle* handle = (OS_WindowHandle*)data;

  OS_Event event = {
    .type = OS_EVENT_TYPE_MOUSE_MOVE,
    .mouse_position = { (F32)wl_fixed_to_double(surface_x), (F32)wl_fixed_to_double(surface_y) }
  };
  OS_EventListPush(&_os_state.mouse_event_list, event);
}

func void
_PointerHandleButton(void* data, wl_pointer* pointer, U32 serial, U32 time, U32 button, U32 state)
{
  OS_WindowHandle* handle = (OS_WindowHandle*)data;

  OS_Event event = {0};
  event.type = OS_EVENT_TYPE_MOUSE_PRESS;
  event.pressed = state == 1;
  event.released = state == 0;
  
  switch (button)
  {
    default: {}; break;

    case 272: {event.mouse_button = OS_MouseButton_Left;}; break;
    case 273: {event.mouse_button = OS_MouseButton_Right;}; break;
  }

  OS_EventListPush(&_os_state.mouse_event_list, event);
  LOG_INFO("Mouse: %d State: %d\n", button, state);
}

func void
_PointerHandleAxis(void*data, wl_pointer* pointer, U32 time, U32 axis, I32 value)
{
}

func void
_PointerHandleFrame(void* data, wl_pointer* pointer)
{
}

struct wl_pointer_listener _pointer_listener = {
  .enter = _PointerHandleEnter,
  .leave = _PointerHandleLeave,
  .motion = _PointerHandleMotion,
  .button = _PointerHandleButton,
  .axis = _PointerHandleAxis,
  .frame = _PointerHandleFrame,
};

func void
_HandleKeyboardKeymap(void* data, struct wl_keyboard* keyboard, U32 format, I32 fd, U32 size)
{
	OS_WindowHandle* handle = (OS_WindowHandle*)data;
	char* keymap_string = mmap(0, size, PROT_READ, MAP_SHARED, fd, 0);
	xkb_keymap_unref(handle->kb_keymap);
	handle->kb_keymap = xkb_keymap_new_from_string(handle->kb_context, keymap_string, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
	munmap(keymap_string, size);
	close(fd);
	xkb_state_unref(handle->kb_state);
	handle->kb_state = xkb_state_new(handle->kb_keymap);
}

func void
_HandleKeyboardEnter(void* data, struct wl_keyboard* keyboard, U32 serial, struct wl_surface* surface, struct wl_array* keys)
{
}

func void
_HandleKeyboardLeave(void* data, struct wl_keyboard* keyboard, U32 serial, struct wl_surface* surface)
{
}

func void
_HandleKeyboardKey(void* data, struct wl_keyboard* keyboard, U32 serial, U32 time, U32 key, U32 state)
{
	OS_WindowHandle* handle = (OS_WindowHandle*)data;

	B32 key_pressed = (state == WL_KEYBOARD_KEY_STATE_PRESSED) || (state == WL_KEYBOARD_KEY_STATE_REPEATED);
	B32 key_released = (state == WL_KEYBOARD_KEY_STATE_RELEASED);
	if (key_pressed || key_released)
	{
		xkb_keysym_t keysym = xkb_state_key_get_one_sym(handle->kb_state, key+8);
		char name[64];
		xkb_keysym_get_name(keysym, name, 64);

		OS_Event event = {0};
		event.type = OS_EVENT_TYPE_KEYBOARD;
		switch (keysym)
		{
			default: {}; break;
			
			case XKB_KEY_Escape: {event.key = OS_KEY_ESC;};break;
			case XKB_KEY_F1: {event.key = OS_KEY_F1;};break;
			case XKB_KEY_F2: {event.key = OS_KEY_F2;};break;
			case XKB_KEY_F3: {event.key = OS_KEY_F3;};break;
			case XKB_KEY_F4: {event.key = OS_KEY_F4;};break;
			case XKB_KEY_F5: {event.key = OS_KEY_F5;};break;
			case XKB_KEY_F6: {event.key = OS_KEY_F6;};break;
			case XKB_KEY_F7: {event.key = OS_KEY_F7;};break;
			case XKB_KEY_F8: {event.key = OS_KEY_F8;};break;
			case XKB_KEY_F9: {event.key = OS_KEY_F9;};break;
			case XKB_KEY_F10: {event.key = OS_KEY_F10;};break;
			case XKB_KEY_F11: {event.key = OS_KEY_F11;};break;
			case XKB_KEY_F12: {event.key = OS_KEY_F12;};break;
			case XKB_KEY_grave: {event.key = OS_KEY_BACKTICK;};break;
			case XKB_KEY_0: {event.key = OS_KEY_0;};break;
			case XKB_KEY_1: {event.key = OS_KEY_1;};break;
			case XKB_KEY_2: {event.key = OS_KEY_2;};break;
			case XKB_KEY_3: {event.key = OS_KEY_3;};break;
			case XKB_KEY_4: {event.key = OS_KEY_4;};break;
			case XKB_KEY_5: {event.key = OS_KEY_5;};break;
			case XKB_KEY_6: {event.key = OS_KEY_6;};break;
			case XKB_KEY_7: {event.key = OS_KEY_7;};break;
			case XKB_KEY_8: {event.key = OS_KEY_8;};break;
			case XKB_KEY_9: {event.key = OS_KEY_9;};break;
			case XKB_KEY_minus: {event.key = OS_KEY_MINUS;};break;
			case XKB_KEY_equal: {event.key = OS_KEY_EQUAL;};break;
			case XKB_KEY_BackSpace: {event.key = OS_KEY_BACKSPACE;};break;
			case XKB_KEY_Tab: {event.key = OS_KEY_TAB;};break;
			case XKB_KEY_q: {event.key = OS_KEY_Q;};break;
			case XKB_KEY_w: {event.key = OS_KEY_W;};break;
			case XKB_KEY_e: {event.key = OS_KEY_E;};break;
			case XKB_KEY_r: {event.key = OS_KEY_R;};break;
			case XKB_KEY_t: {event.key = OS_KEY_T;};break;
			case XKB_KEY_y: {event.key = OS_KEY_Y;};break;
			case XKB_KEY_u: {event.key = OS_KEY_U;};break;
			case XKB_KEY_i: {event.key = OS_KEY_I;};break;
			case XKB_KEY_o: {event.key = OS_KEY_O;};break;
			case XKB_KEY_p: {event.key = OS_KEY_P;};break;
			case XKB_KEY_bracketleft: {event.key = OS_KEY_LEFT_BRACKET;};break;
			case XKB_KEY_bracketright: {event.key = OS_KEY_RIGHT_BRACKET;};break;
			case XKB_KEY_backslash: {event.key = OS_KEY_BACK_SLASH;};break;
			case XKB_KEY_Caps_Lock: {event.key = OS_KEY_CAPS_LOCK;};break;
			case XKB_KEY_a: {event.key = OS_KEY_A;};break;
			case XKB_KEY_s: {event.key = OS_KEY_S;};break;
			case XKB_KEY_d: {event.key = OS_KEY_D;};break;
			case XKB_KEY_f: {event.key = OS_KEY_F;};break;
			case XKB_KEY_g: {event.key = OS_KEY_G;};break;
			case XKB_KEY_h: {event.key = OS_KEY_H;};break;
			case XKB_KEY_j: {event.key = OS_KEY_J;};break;
			case XKB_KEY_k: {event.key = OS_KEY_K;};break;
			case XKB_KEY_l: {event.key = OS_KEY_L;};break;
			case XKB_KEY_semicolon: {event.key = OS_KEY_SEMICOLON;};break;
			case XKB_KEY_apostrophe: {event.key = OS_KEY_QUOTE;};break;
			case XKB_KEY_Return: {event.key = OS_KEY_RETURN;};break;
			case XKB_KEY_Shift_L: {event.key = OS_KEY_SHIFT;};break;
			case XKB_KEY_Shift_R: {event.key = OS_KEY_SHIFT;};break;
			case XKB_KEY_z: {event.key = OS_KEY_Z;};break;
			case XKB_KEY_x: {event.key = OS_KEY_X;};break;
			case XKB_KEY_c: {event.key = OS_KEY_C;};break;
			case XKB_KEY_v: {event.key = OS_KEY_V;};break;
			case XKB_KEY_b: {event.key = OS_KEY_B;};break;
			case XKB_KEY_n: {event.key = OS_KEY_N;};break;
			case XKB_KEY_m: {event.key = OS_KEY_M;};break;
			case XKB_KEY_comma: {event.key = OS_KEY_COMMA;};break;
			case XKB_KEY_period: {event.key = OS_KEY_PERIOD;};break;
			case XKB_KEY_slash: {event.key = OS_KEY_SLASH;};break;
			case XKB_KEY_Control_L: {event.key = OS_KEY_CTRL;};break;
			case XKB_KEY_Control_R: {event.key = OS_KEY_CTRL;};break;
			case XKB_KEY_Alt_L: {event.key = OS_KEY_ALT;};break;
			case XKB_KEY_Alt_R: {event.key = OS_KEY_ALT;};break;
			case XKB_KEY_space: {event.key = OS_KEY_SPACE;};break;
			case XKB_KEY_Up: {event.key = OS_KEY_ARROW_UP;};break;
			case XKB_KEY_Down: {event.key = OS_KEY_ARROW_DOWN;};break;
			case XKB_KEY_Left: {event.key = OS_KEY_ARROW_LEFT;};break;
			case XKB_KEY_Right: {event.key = OS_KEY_ARROW_RIGHT;};break;
		}

		if (key_pressed)
		{
			LOG_DEBUG("(XKB) %s is pressed\n", name);
		}
		if (key_released)
		{
			LOG_DEBUG("(XKB) %s is released\n", name);
		}

		event.pressed = key_pressed;
		event.released = key_released;

		OS_EventListPush(&_os_state.keyboard_event_list, event);
	}
}

func void
_HandleKeyboardModifiers(void* data, struct wl_keyboard* keyboard, U32 serial, U32 mods_depressed, U32 mods_latched, U32 mods_locked, U32 group)
{
	OS_WindowHandle* handle = (OS_WindowHandle*)data;

	xkb_state_update_mask(handle->kb_state, mods_depressed, mods_latched, mods_locked, 0, 0, group);
}

func void
_HandleKeyboardRepeat(void* data, struct wl_keyboard* keyboard, I32 rate, I32 delay)
{
	LOG_DEBUG("REPEAT INFO\n");
}

struct wl_keyboard_listener _keyboard_listener = {
	.keymap = _HandleKeyboardKeymap,
	.enter = _HandleKeyboardEnter,
	.leave = _HandleKeyboardLeave,
	.key = _HandleKeyboardKey,
	.modifiers = _HandleKeyboardModifiers,
	.repeat_info = _HandleKeyboardRepeat,
};

func void
_SeatHandleCapabilities(void* data, wl_seat* seat, U32 capabilities)
{
  OS_WindowHandle* handle = (OS_WindowHandle*)data;

  B32 have_pointer = capabilities & WL_SEAT_CAPABILITY_POINTER;

  if (have_pointer && handle->pointer == 0)
  {
    handle->pointer = wl_seat_get_pointer(handle->seat);
    wl_pointer_add_listener(handle->pointer, &_pointer_listener, data);
    LOG_INFO("ADD POINTER\n");
  }
  else if (!have_pointer && handle->pointer !=0)
  {
    wl_pointer_release(handle->pointer);
    handle->pointer = 0;
  }
	if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD)
	{
		handle->keyboard = wl_seat_get_keyboard(handle->seat);
		wl_keyboard_add_listener(handle->keyboard, &_keyboard_listener, data);
	}
}

func void
_SeatHandleName(void* data, wl_seat* seat, const char* name)
{
  LOG_INFO("WL_Seat name: %s\n");
}

struct wl_seat_listener _seat_listener = {
  .capabilities = _SeatHandleCapabilities,
  .name = _SeatHandleName
};

struct xdg_wm_base_listener _shell_listener = {
  .ping = _ShellHandlePing
};

func void
_RegistryHandleGlobal(void* data, wl_registry* registry, U32 name, const char* interface, U32 version)
{
  OS_WindowHandle* handle = (OS_WindowHandle*)data;

  if (strcmp(interface, wl_compositor_interface.name) == 0)
  {
    handle->compositor = (wl_compositor*)wl_registry_bind(registry, name, &wl_compositor_interface, 1);
  }
  else if (strcmp(interface, xdg_wm_base_interface.name) == 0)
  {
    handle->shell = (xdg_wm_base*)wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
    xdg_wm_base_add_listener(handle->shell, &_shell_listener, handle);
  }
  else if (strcmp(interface, wl_seat_interface.name) == 0)
  {
    handle->seat = (wl_seat*)wl_registry_bind(registry, name, &wl_seat_interface, 7);
    wl_seat_add_listener(handle->seat, &_seat_listener, handle);
  }
  else if (strcmp(interface, zwp_relative_pointer_manager_v1_interface.name) == 0)
  {
    handle->relative_pointer_manager = (zwp_relative_pointer_manager_v1*)wl_registry_bind(registry, name, &zwp_relative_pointer_manager_v1_interface, 1);
  }
  else if (strcmp(interface, zwp_pointer_constraints_v1_interface.name) == 0)
  {
    handle->pointer_constraints = (zwp_pointer_constraints_v1*)wl_registry_bind(registry, name, &zwp_pointer_constraints_v1_interface, 1);
  }
}

struct wl_registry_listener _registry_listener = {
  .global = _RegistryHandleGlobal,
  .global_remove = 0
};

func void
_ShellSurfaceHandleConfigure(void* data, xdg_surface* shell_surface, U32 serial)
{
  OS_WindowHandle* handle = (OS_WindowHandle*)data;

  xdg_surface_ack_configure(shell_surface, serial);

  if (handle->request_resize)
  {
    OS_Event event = {
      .type = OS_EVENT_TYPE_RESIZE,
      .window_size = handle->new_size,
    };

    // --AlNov: @NOTE Configure occures before GetEventList, so event_list is not initialized
    if (_os_state.event_list.arena)
    {
      OS_EventListPush(&_os_state.event_list, event);
    }

    handle->request_resize = false;
    wl_surface_commit(handle->surface);
  }
}

struct xdg_surface_listener _shell_surface_listener = {
  .configure = _ShellSurfaceHandleConfigure
};

func void
_ToplevelHandleConfigure(void* data, xdg_toplevel* toplevel, I32 new_width, I32 new_heigth, struct wl_array* states)
{
  OS_WindowHandle* handle = (OS_WindowHandle*)data;

  handle->new_size = MakeVec2U32((I32)new_width, (I32)new_heigth);
  handle->request_resize = true;
}

func void
_ToplevelHandleClose(void* data, xdg_toplevel* toplevel)
{
  LOG_INFO("CLOSE\n");
  OS_Event event = {
    .type = OS_EVENT_TYPE_EXIT,
  };

  OS_EventListPush(&_os_state.event_list, event);
}

struct xdg_toplevel_listener _toplevel_listener = {
  .configure = _ToplevelHandleConfigure,
  .close = _ToplevelHandleClose
};

func void
_HandleRelativeMotion(void* data, zwp_relative_pointer_v1* pointer, U32 time_hi, U32 time_lo, I32 dx, I32 dy, I32 dx_unaccel, I32 dy_unaccel)
{
  OS_Window* window = (OS_Window*)data;
  window->virtual_cursor_position = AddVec2F32(window->virtual_cursor_position, MakeVec2F32(wl_fixed_to_double(dx), wl_fixed_to_double(dy)));
}

struct zwp_relative_pointer_v1_listener _relative_pointer_listener = {
  .relative_motion = _HandleRelativeMotion
};

func void
OS_CreateWindow(Str8 title, Vec2U32 size, OS_Window* out)
{
  out->handle = (OS_WindowHandle*)OS_AllocateMemory(sizeof(OS_WindowHandle));
  out->size = size;

  out->handle->display = wl_display_connect(0);
  out->handle->registry = wl_display_get_registry(out->handle->display);
  wl_registry_add_listener(out->handle->registry, &_registry_listener, out->handle);
  wl_display_roundtrip(out->handle->display);

  out->handle->surface = wl_compositor_create_surface(out->handle->compositor);
  out->handle->shell_surface = xdg_wm_base_get_xdg_surface(out->handle->shell, out->handle->surface);
  out->handle->toplevel = xdg_surface_get_toplevel(out->handle->shell_surface);
  xdg_surface_add_listener(out->handle->shell_surface, &_shell_surface_listener, out->handle);
  xdg_toplevel_add_listener(out->handle->toplevel, &_toplevel_listener, out->handle);

  xdg_toplevel_set_title(out->handle->toplevel, CFromStr8(title));
  xdg_toplevel_set_app_id(out->handle->toplevel, CFromStr8(title));

  wl_surface_commit(out->handle->surface);
  wl_display_roundtrip(out->handle->display);
  wl_surface_commit(out->handle->surface);

  if (!out->handle->relative_pointer_manager)
  {
    LOG_ERROR("Relative Pointer is not supported by the compositor.\n");
    return;
  }

  out->handle->relative_pointer = zwp_relative_pointer_manager_v1_get_relative_pointer(
    out->handle->relative_pointer_manager,
    out->handle->pointer
  );
  zwp_relative_pointer_v1_add_listener(
    out->handle->relative_pointer,
    &_relative_pointer_listener,
    out
  );

	out->handle->kb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

  LOG_INFO("Window Created\n");
}

func void
OS_ShowWindow(OS_Window* window)
{
  LOG_INFO("Show Window\n");
}

func Vec2F32
OS_MousePosition(OS_Window window)
{
  return window.cursor_position;
}

func void
_LockedPointerHandleLocked(void* data, zwp_locked_pointer_v1* pointer)
{
  LOG_INFO("Pointer is locked.\n")
}

func void
_LockedPointerHandleUnlocked(void* data, zwp_locked_pointer_v1* pointer)
{
  LOG_INFO("Pointer is unlocked.\n");
}

struct zwp_locked_pointer_v1_listener _locked_pointer_listener = {
  .locked = _LockedPointerHandleLocked,
  .unlocked = _LockedPointerHandleUnlocked
};

func void
_ConfinedPointerHandleConfined(void* data, zwp_confined_pointer_v1* pointer)
{
  
  LOG_INFO("Pointer is confined.\n");
}

func void
_ConfinedPointerHandleUnconfined(void* data, zwp_confined_pointer_v1* pointer)
{
  LOG_INFO("Pointer is unconfined.\n");
}

struct zwp_confined_pointer_v1_listener _confined_pointer_listener = {
  .confined = _ConfinedPointerHandleConfined,
  .unconfined = _ConfinedPointerHandleUnconfined
};

func void
OS_LockCursor(OS_Window* window)
{
  window->handle->confined_pointer = zwp_pointer_constraints_v1_confine_pointer(
    window->handle->pointer_constraints,
    window->handle->surface,
    window->handle->pointer,
    0,
    ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT
  );
  zwp_confined_pointer_v1_add_listener(window->handle->confined_pointer, &_confined_pointer_listener, window);

  // window->handle->locked_pointer = zwp_pointer_constraints_v1_lock_pointer(
  //   window->handle->pointer_constraints,
  //   window->handle->surface,
  //   window->handle->pointer,
  //   0,
  //   ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT
  // );
  // zwp_locked_pointer_v1_add_listener(window->handle->locked_pointer, &_locked_pointer_listener, window);
}

func void
OS_UnlockCursor(OS_Window* window)
{
  zwp_relative_pointer_v1_destroy(window->handle->relative_pointer);
  window->handle->relative_pointer = 0;

  // zwp_locked_pointer_v1_destroy(window->handle->locked_pointer);
  // window->handle->locked_pointer = 0;

  zwp_confined_pointer_v1_destroy(window->handle->confined_pointer);
  window->handle->confined_pointer = 0;
}

func OS_EventList
OS_GetEventList(Arena* arena, OS_Window* window)
{
  _os_state.event_list = OS_EventListCreate(arena);
	_os_state.keyboard_event_list = OS_EventListCreate(arena);
  _os_state.mouse_event_list = OS_EventListCreate(arena);

  wl_display_dispatch_pending(window->handle->display);

	for (I32 i = 0; i < OS_KEY_COUNT; i += 1)
	{
		OS_KeyState* key = _os_state.keyboard.keys + i;
		key->pressed = 0;
		key->released = 0;
		key->time_down += key->is_down * (0.0f); // AlNov: @TODO Add delta time
	}

  for (OS_EventListNode *event_node = _os_state.keyboard_event_list.first; event_node; event_node = event_node->next)
	{
		OS_Event* event = &event_node->data;
		_os_state.keyboard.keys[event->key].pressed = event->pressed;
		_os_state.keyboard.keys[event->key].released = !event->pressed;
		if (event->pressed)
		{
			_os_state.keyboard.keys[event->key].is_down = 1;
		}
		if (event->released)
		{
			_os_state.keyboard.keys[event->key].is_down = 0;
		}
	}

  for (I32 i = 0; i < OS_MouseButton_Count; i += 1)
  {
    OS_MouseButtonState* button = _os_state.mouse.buttons + i;
    button->pressed = 0;
    button->released = 0;
  }

  for (OS_EventListNode* event_node = _os_state.mouse_event_list.first; event_node; event_node = event_node->next)
  {
    OS_Event* event = &event_node->data;
    if (event->type == OS_EVENT_TYPE_MOUSE_MOVE)
    {
      window->cursor_position = event->mouse_position;
    }
    else
    {
      _os_state.mouse.buttons[event->mouse_button].pressed = event->pressed;
      _os_state.mouse.buttons[event->mouse_button].released = event->released;
      if (event->pressed)
      {
        _os_state.mouse.buttons[event->mouse_button].is_down = 1;
      }
      if (event->released)
      {
        _os_state.mouse.buttons[event->mouse_button].is_down = 0;
      }
    }
  }

  return _os_state.event_list;
}

func U64
OS_GetTimeTicks(void)
{
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC_RAW, &now);

  return (U64)(now.tv_sec*1000 + now.tv_nsec*0.000001);
}
