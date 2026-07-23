#pragma once

#include "os_macos_gfx.h"

func void
OS_Init(U64 arena_size) {
  _os_state.arena = AllocateArena(Gigabytes(8), arena_size);
  NSApplication* ns_app = [NSApplication sharedApplication];
  _os_macos_state.ns_app = (__bridge void*)ns_app;

  [ns_app setActivationPolicy:NSApplicationActivationPolicyRegular];
  [ns_app finishLaunching];
}

@implementation OS_MacOS_WindowDelegate
- (void)windowWillClose:(NSNotification*) notification {
  OS_Event event = {
    .type = OS_EVENT_TYPE_EXIT,
  };
  // -AlNov 23.07.2026: @TODO Event list created in DispatchEvent. So we could not be sure it available here
  // But it is working for now
  OS_EventListPush(&_os_state.event_list, event);
}

- (void)windowDidClose:(NSNotification*) notification {
  LogDebug("Macos WindowDidClose\n");
}
@end

@implementation OS_MacOS_View
+ (Class)layerClass {
  return NSClassFromString(@"CAMetalLayer");
}

- (BOOL)wantsUpdateLayer {
  return YES;
}

- (CALayer*)makeBackingLayer {
  return [self.class.layerClass layer];
}

- (instancetype)initWithFrame:(NSRect)frame {
  self = [super initWithFrame:frame];
  Assert(self);

  self.autoresizingMask = NSViewWidthSizable|NSViewHeightSizable;
  self.wantsLayer = YES;
  self.metal_layer = (CAMetalLayer*)[self layer];

  return self;
}

- (CAMetalLayer*)MetalLayer {
  return self.metal_layer;
}
@end

func OS_Window*
OS_CreateWindow(Str8 title, Vec2U32 size) {
  // --AlNov: @TODO should use allocator
  OS_MacOS_Window* window = (OS_MacOS_Window*)PushArena(_os_state.arena, sizeof(OS_MacOS_Window));
  window->header.size = size;

  NSRect screen_rect = [[NSScreen mainScreen] frame];
  NSRect window_rect = NSMakeRect((screen_rect.size.width - size.w)*0.5f, (screen_rect.size.height - size.h)*0.5f, size.w, size.h);
  NSWindow* ns_window = nil;
  ns_window = [[NSWindow alloc]
    initWithContentRect:window_rect
    styleMask:NSWindowStyleMaskTitled|NSWindowStyleMaskClosable|NSWindowStyleMaskMiniaturizable|NSWindowStyleMaskResizable
    backing:NSBackingStoreBuffered
    defer:NO
  ];

  Assert(ns_window != nil);
  window->ns_window = (__bridge void*)ns_window;

  [ns_window setTitle:@(CFromStr8(title))];

  OS_MacOS_WindowDelegate* ns_delegate = [[OS_MacOS_WindowDelegate alloc] init];
  ns_window.delegate = ns_delegate;

  OS_MacOS_View* view = [[OS_MacOS_View alloc] initWithFrame:NSMakeRect(0, 0, size.w, size.h)];
  window->ns_view = (__bridge void*)view;
  [ns_window setContentView: view];

  return (OS_Window*)window;
}

func void
OS_DestroyWindow(OS_Window* window) {
  // --AlNov: @TODO
}

func void
OS_ShowWindow(OS_Window* window) {
  OS_MacOS_Window* macos_window = (OS_MacOS_Window*)window;
  NSWindow* ns_window = (__bridge NSWindow*)macos_window->ns_window;
  [ns_window makeKeyAndOrderFront:nil];
}

func void
OS_LockCursor(OS_Window* window) {
}

func void
OS_UnlockCursor(OS_Window* window) {
}

func OS_EventList
OS_DispatchEvents(Arena* arena, OS_Window* window) {
  _os_state.event_list = OS_EventListCreate(arena);
  _os_state.keyboard_event_list = OS_EventListCreate(arena);
  _os_state.mouse_event_list = OS_EventListCreate(arena);
  _os_state.mouse.scroll = MakeVec2F32(0.0f, 0.0f);

  NSEvent* ns_event = nil;
  do {
    ns_event = [
      NSApp nextEventMatchingMask: NSEventMaskAny
      untilDate:nil
      inMode:NSDefaultRunLoopMode
      dequeue: YES
    ];

    OS_Event event = ZeroStruct();

    switch ([ns_event type]) {
      default: {} break;

      case NSEventTypeKeyDown:
      case NSEventTypeKeyUp: {
        event.type = OS_EVENT_TYPE_KEYBOARD;
        event.pressed = [ns_event type] == NSEventTypeKeyDown;
        event.released = [ns_event type] == NSEventTypeKeyUp;

        switch ([ns_event keyCode]) {
          case 53 : {event.key = OS_KEY_ESC;};break;
          case 122: {event.key = OS_KEY_F1;};break;
          case 120: {event.key = OS_KEY_F2;};break;
          case 99 : {event.key = OS_KEY_F3;};break;
          case 118: {event.key = OS_KEY_F4;};break;
          case 96 : {event.key = OS_KEY_F5;};break;
          case 97 : {event.key = OS_KEY_F6;};break;
          case 98 : {event.key = OS_KEY_F7;};break;
          case 100: {event.key = OS_KEY_F8;};break;
          case 101: {event.key = OS_KEY_F9;};break;
          case 109: {event.key = OS_KEY_F10;};break;
          case 103: {event.key = OS_KEY_F11;};break;
          case 111: {event.key = OS_KEY_F12;};break;
          case 50 : {event.key = OS_KEY_BACKTICK;};break;
          case 29 : {event.key = OS_KEY_0;};break;
          case 18 : {event.key = OS_KEY_1;};break;
          case 19 : {event.key = OS_KEY_2;};break;
          case 20 : {event.key = OS_KEY_3;};break;
          case 21 : {event.key = OS_KEY_4;};break;
          case 23 : {event.key = OS_KEY_5;};break;
          case 22 : {event.key = OS_KEY_6;};break;
          case 26 : {event.key = OS_KEY_7;};break;
          case 28 : {event.key = OS_KEY_8;};break;
          case 25 : {event.key = OS_KEY_9;};break;
          case 27 : {event.key = OS_KEY_MINUS;};break;
          case 24 : {event.key = OS_KEY_EQUAL;};break;
          case 51 : {event.key = OS_KEY_BACKSPACE;};break;
          case 48 : {event.key = OS_KEY_TAB;};break;
          case 12 : {event.key = OS_KEY_Q;};break;
          case 13 : {event.key = OS_KEY_W;};break;
          case 14 : {event.key = OS_KEY_E;};break;
          case 15 : {event.key = OS_KEY_R;};break;
          case 17 : {event.key = OS_KEY_T;};break;
          case 16 : {event.key = OS_KEY_Y;};break;
          case 32 : {event.key = OS_KEY_U;};break;
          case 34 : {event.key = OS_KEY_I;};break;
          case 31 : {event.key = OS_KEY_O;};break;
          case 35 : {event.key = OS_KEY_P;};break;
          case 33 : {event.key = OS_KEY_LEFT_BRACKET;};break;
          case 30 : {event.key = OS_KEY_RIGHT_BRACKET;};break;
          case 42 : {event.key = OS_KEY_BACK_SLASH;};break;
          case 57 : {event.key = OS_KEY_CAPS_LOCK;};break;
          case 0  : {event.key = OS_KEY_A;};break;
          case 1  : {event.key = OS_KEY_S;};break;
          case 2  : {event.key = OS_KEY_D;};break;
          case 3  : {event.key = OS_KEY_F;};break;
          case 5  : {event.key = OS_KEY_G;};break;
          case 4  : {event.key = OS_KEY_H;};break;
          case 38 : {event.key = OS_KEY_J;};break;
          case 40 : {event.key = OS_KEY_K;};break;
          case 37 : {event.key = OS_KEY_L;};break;
          case 41 : {event.key = OS_KEY_SEMICOLON;};break;
          case 39 : {event.key = OS_KEY_QUOTE;};break;
          case 36 : {event.key = OS_KEY_RETURN;};break;
          case 56 : {event.key = OS_KEY_SHIFT;};break; // Left
          case 69 : {event.key = OS_KEY_SHIFT;};break; // Right
          case 6  : {event.key = OS_KEY_Z;};break;
          case 7  : {event.key = OS_KEY_X;};break;
          case 8  : {event.key = OS_KEY_C;};break;
          case 9  : {event.key = OS_KEY_V;};break;
          case 11 : {event.key = OS_KEY_B;};break;
          case 45 : {event.key = OS_KEY_N;};break;
          case 46 : {event.key = OS_KEY_M;};break;
          case 43 : {event.key = OS_KEY_COMMA;};break;
          case 47 : {event.key = OS_KEY_PERIOD;};break;
          case 44 : {event.key = OS_KEY_SLASH;};break;
          case 59 : {event.key = OS_KEY_CTRL;};break; // Left
          case 62 : {event.key = OS_KEY_CTRL;};break; // Right
          case 58 : {event.key = OS_KEY_ALT;};break; // Left
          case 61 : {event.key = OS_KEY_ALT;};break; // Right
          case 49 : {event.key = OS_KEY_SPACE;};break;
          case 126: {event.key = OS_KEY_ARROW_UP;};break;
          case 125: {event.key = OS_KEY_ARROW_DOWN;};break;
          case 123: {event.key = OS_KEY_ARROW_LEFT;};break;
          case 124: {event.key = OS_KEY_ARROW_RIGHT;};break;
        }
      } break;

      case NSEventTypeLeftMouseDown: {
        event.type = OS_EVENT_TYPE_MOUSE_PRESS;
        event.pressed = 1;
        event.mouse_button = OS_MouseButton_Left;
      } break;

      case NSEventTypeLeftMouseUp: {
        event.type = OS_EVENT_TYPE_MOUSE_PRESS;
        event.released = 1;
        event.mouse_button = OS_MouseButton_Left;
      } break;

      case NSEventTypeScrollWheel: {
        if ([ns_event hasPreciseScrollingDeltas]) {
          _os_state.mouse.scroll = MakeVec2F32([ns_event scrollingDeltaX], [ns_event scrollingDeltaY]);
        }
        else {
          _os_state.mouse.scroll = MakeVec2F32([ns_event deltaX], [ns_event deltaY]);
        }
      } break;
    }

    if (event.type != 0) {
      if (event.type == OS_EVENT_TYPE_KEYBOARD) {
        OS_EventListPush(&_os_state.keyboard_event_list, event);
      } else if (event.type == OS_EVENT_TYPE_MOUSE_PRESS) {
        OS_EventListPush(&_os_state.mouse_event_list, event);
      }
    }
    [NSApp sendEvent: ns_event];
  } while (ns_event != nil);

  // --AlNov: @TODO It is repeated in every OS
  for (I32 i = 0; i < OS_KEY_COUNT; i += 1) {
    OS_KeyState* key = _os_state.keyboard.keys + i;
    key->pressed = 0;
    key->released = 0;
    key->time_down += key->is_down * 0.0f;
  }

  for (OS_EventListNode *event_node = _os_state.keyboard_event_list.first; event_node; event_node = event_node->next) {

		OS_Event* event = &event_node->data;
		_os_state.keyboard.keys[event->key].pressed = event->pressed;
		_os_state.keyboard.keys[event->key].released = !event->pressed;
		if (event->pressed) {
			_os_state.keyboard.keys[event->key].is_down = 1;
		}
		if (event->released) {
			_os_state.keyboard.keys[event->key].is_down = 0;
		}
	}

  for (I32 i = 0; i < OS_MouseButton_Count; i += 1) {
    OS_MouseButtonState* button = _os_state.mouse.buttons + i;
    button->pressed = 0;
    button->released = 0;
  }

  for (OS_EventListNode* event_node = _os_state.mouse_event_list.first; event_node; event_node = event_node->next) {
    OS_Event* event = &event_node->data;
    _os_state.mouse.buttons[event->mouse_button].pressed = event->pressed;
    _os_state.mouse.buttons[event->mouse_button].released = event->released;
    if (event->pressed) {
      _os_state.mouse.buttons[event->mouse_button].is_down = 1;
    }
    if (event->released) {
      _os_state.mouse.buttons[event->mouse_button].is_down = 0;
    }
  }

  return _os_state.event_list;
}

func F32 OS_GetMonitorHZ(void);
func U64 OS_GetTimeTicks(void) {
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC_RAW, &now);

  return (U64)(now.tv_sec*1000 + now.tv_nsec*0.000001);
}

func void
OS_Sleep(U64 ms) {
  sleep((F32)(ms)/1000.0f);
}

func Vec2F32
OS_MousePosition(OS_Window* window) {
   OS_MacOS_Window* macos_window = (OS_MacOS_Window*)window; 
   NSWindow* ns_window = (__bridge NSWindow*)macos_window->ns_window;
   NSPoint point = [ns_window mouseLocationOutsideOfEventStream];

   return MakeVec2F32(point.x, window->size.h - point.y);
}

func Vec2F32
OS_MouseScroll() {
  return _os_state.mouse.scroll;
}
