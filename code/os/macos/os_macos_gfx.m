#pragma once

#include "os_macos_gfx.h"

func void
OS_Init(U64 arena_size) {
  @autoreleasepool {
    _os_state.arena = AllocateArena(Gigabytes(8), arena_size);
    _os_state.event_arena = AllocateArena(Gigabytes(1), Kilobytes(16));
    NSApplication* ns_app = [NSApplication sharedApplication];
    _os_macos_state.ns_app = (__bridge void*)ns_app;

    [ns_app setActivationPolicy:NSApplicationActivationPolicyRegular];
    [ns_app finishLaunching];
  }
}

@implementation OS_MacOS_WindowDelegate
- (void)windowWillClose:(NSNotification*) notification {
  OS_Event* event = (OS_Event*)PushArena(_os_state.event_arena, sizeof(OS_Event));
  event->kind = OS_EventKind_Exit;
  SllPushBack(_os_state.event_list.first, _os_state.event_list.last, event);
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
  @autoreleasepool {
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
  window->ns_window = (__bridge_retained void*)ns_window;

  [ns_window setTitle:@(CFromStr8(title))];

  OS_MacOS_WindowDelegate* ns_delegate = [[OS_MacOS_WindowDelegate alloc] init];
  ns_window.delegate = ns_delegate;

  OS_MacOS_View* view = [[OS_MacOS_View alloc] initWithFrame:NSMakeRect(0, 0, size.w, size.h)];
  window->ns_view = (__bridge void*)view;
  [ns_window setContentView: view];

  return (OS_Window*)window;
  }
}

func void
OS_DestroyWindow(OS_Window* window) {
  @autoreleasepool {
    OS_MacOS_Window* macos_window = (OS_MacOS_Window*)window;
    NSWindow* ns_window = (__bridge_transfer NSWindow*)macos_window->ns_window;
    [ns_window close];

    *macos_window = (OS_MacOS_Window)ZeroStruct();
  }
}

func void
OS_ShowWindow(OS_Window* window) {
  @autoreleasepool {
    OS_MacOS_Window* macos_window = (OS_MacOS_Window*)window;
    NSWindow* ns_window = (__bridge NSWindow*)macos_window->ns_window;
    [ns_window makeKeyAndOrderFront:nil];
  }
}

func void
OS_LockCursor(OS_Window* window) {
}

func void
OS_UnlockCursor(OS_Window* window) {
}

func void
OS_ShowCursor(B32 to_show) {
}

OS_KeyCode
OS_KeyCodeFromMacOS(U32 mac_key) {
  OS_KeyCode result = OS_Key_None;
  switch (mac_key) {
    case 53 : {result = OS_Key_Esc;};break;
    case 122: {result = OS_Key_F1;};break;
    case 120: {result = OS_Key_F2;};break;
    case 99 : {result = OS_Key_F3;};break;
    case 118: {result = OS_Key_F4;};break;
    case 96 : {result = OS_Key_F5;};break;
    case 97 : {result = OS_Key_F6;};break;
    case 98 : {result = OS_Key_F7;};break;
    case 100: {result = OS_Key_F8;};break;
    case 101: {result = OS_Key_F9;};break;
    case 109: {result = OS_Key_F10;};break;
    case 103: {result = OS_Key_F11;};break;
    case 111: {result = OS_Key_F12;};break;
    case 50 : {result = OS_Key_Backtick;};break;
    case 29 : {result = OS_Key_0;};break;
    case 18 : {result = OS_Key_1;};break;
    case 19 : {result = OS_Key_2;};break;
    case 20 : {result = OS_Key_3;};break;
    case 21 : {result = OS_Key_4;};break;
    case 23 : {result = OS_Key_5;};break;
    case 22 : {result = OS_Key_6;};break;
    case 26 : {result = OS_Key_7;};break;
    case 28 : {result = OS_Key_8;};break;
    case 25 : {result = OS_Key_9;};break;
    case 27 : {result = OS_Key_Minus;};break;
    case 24 : {result = OS_Key_Equal;};break;
    case 51 : {result = OS_Key_Backspace;};break;
    case 48 : {result = OS_Key_Tab;};break;
    case 12 : {result = OS_Key_Q;};break;
    case 13 : {result = OS_Key_W;};break;
    case 14 : {result = OS_Key_E;};break;
    case 15 : {result = OS_Key_R;};break;
    case 17 : {result = OS_Key_T;};break;
    case 16 : {result = OS_Key_Y;};break;
    case 32 : {result = OS_Key_U;};break;
    case 34 : {result = OS_Key_I;};break;
    case 31 : {result = OS_Key_O;};break;
    case 35 : {result = OS_Key_P;};break;
    case 33 : {result = OS_Key_LeftBracket;};break;
    case 30 : {result = OS_Key_RightBracket;};break;
    case 42 : {result = OS_Key_BackSlash;};break;
    case 57 : {result = OS_Key_CapsLock;};break;
    case 0  : {result = OS_Key_A;};break;
    case 1  : {result = OS_Key_S;};break;
    case 2  : {result = OS_Key_D;};break;
    case 3  : {result = OS_Key_F;};break;
    case 5  : {result = OS_Key_G;};break;
    case 4  : {result = OS_Key_H;};break;
    case 38 : {result = OS_Key_J;};break;
    case 40 : {result = OS_Key_K;};break;
    case 37 : {result = OS_Key_L;};break;
    case 41 : {result = OS_Key_Semicolon;};break;
    case 39 : {result = OS_Key_Quote;};break;
    case 36 : {result = OS_Key_Return;};break;
    case 56 : {result = OS_Key_Shift;};break; // Left
    case 69 : {result = OS_Key_Shift;};break; // Right
    case 6  : {result = OS_Key_Z;};break;
    case 7  : {result = OS_Key_X;};break;
    case 8  : {result = OS_Key_C;};break;
    case 9  : {result = OS_Key_V;};break;
    case 11 : {result = OS_Key_B;};break;
    case 45 : {result = OS_Key_N;};break;
    case 46 : {result = OS_Key_M;};break;
    case 43 : {result = OS_Key_Comma;};break;
    case 47 : {result = OS_Key_Period;};break;
    case 44 : {result = OS_Key_Slash;};break;
    case 59 : {result = OS_Key_Ctrl;};break; // Left
    case 62 : {result = OS_Key_Ctrl;};break; // Right
    case 58 : {result = OS_Key_Alt;};break; // Left
    case 61 : {result = OS_Key_Alt;};break; // Right
    case 49 : {result = OS_Key_Space;};break;
    case 126: {result = OS_Key_ArrowUp;};break;
    case 125: {result = OS_Key_ArrowDown;};break;
    case 123: {result = OS_Key_ArrowLeft;};break;
    case 124: {result = OS_Key_ArrowRight;};break;
  }
  return result;
}

func OS_EventList
OS_DispatchEvents(OS_Window* window) {
  @autoreleasepool {
    ResetArena(_os_state.event_arena);

    _os_state.event_list.first = 0;
    _os_state.event_list.last = 0;

    memcpy(_os_state.keyboard.previous, _os_state.keyboard.current, sizeof(_os_state.keyboard.previous));
    memcpy(_os_state.mouse.previous, _os_state.mouse.current, sizeof(_os_state.mouse.previous));

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
        default: {
          [NSApp sendEvent: ns_event];
        } break;

        case NSEventTypeKeyDown:
        case NSEventTypeKeyUp: {
          OS_KeyCode key = OS_KeyCodeFromMacOS([ns_event keyCode]);
          _os_state.keyboard.current[key].pressed = [ns_event type] == NSEventTypeKeyDown;
        } break;

        case NSEventTypeLeftMouseDown: {
          _os_state.mouse.current[OS_MouseButton_Left].pressed = 1;
        } break;

        case NSEventTypeLeftMouseUp: {
          _os_state.mouse.current[OS_MouseButton_Left].pressed = 0;
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

      if (event.kind != 0) {
        OS_Event* new_event = (OS_Event*)PushArena(_os_state.event_arena, sizeof(new_event));
        *new_event = event;
        SllPushBack(_os_state.event_list.first, _os_state.event_list.last, new_event);
      }
    } while (ns_event != nil);

    return _os_state.event_list;
  }
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
  @autoreleasepool {
     OS_MacOS_Window* macos_window = (OS_MacOS_Window*)window; 
     NSWindow* ns_window = (__bridge NSWindow*)macos_window->ns_window;
     NSPoint point = [ns_window mouseLocationOutsideOfEventStream];

     return MakeVec2F32(point.x, window->size.h - point.y);
   }
}

func void
OS_SetMousePosition(OS_Window* window, Vec2F32 position) {
}

func Vec2F32
OS_MouseScroll() {
  return _os_state.mouse.scroll;
}
