#include "base/base_include.h"

#include "os/os_gfx.h"

// --AlNov: @NOTE Have to do trick with push/pop macro (not in C standard, but de facto standard in compilers)
#pragma push_macro("func")
#undef func
#include <Cocoa/Cocoa.h>
#pragma pop_macro("func")

typedef struct OS_MacOS OS_MacOS;
struct OS_MacOS {
  void* ns_app;
} _os_macos_state;

func void
OS_Init(U64 arena_size) {
  _os_state.arena = AllocateArena(Gigabytes(8), arena_size);
  NSApplication* ns_app = [NSApplication sharedApplication];
  _os_macos_state.ns_app = (__bridge void*)ns_app;

  [ns_app setActivationPolicy:NSApplicationActivationPolicyRegular];
  [ns_app finishLaunching];
}

typedef struct OS_MacOS_Window OS_MacOS_Window;
struct OS_MacOS_Window {
  OS_Window header;
  void* ns_window;
};

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

func OS_EventList
OS_GetEventList(Arena* arena, OS_Window* window) {
  NSEvent* event = nil;
  do {
    event = [
      NSApp nextEventMatchingMask: NSEventMaskAny
      untilDate:nil
      inMode:NSDefaultRunLoopMode
      dequeue: YES
    ];

    switch ([event type]) {
      default: {
        [NSApp sendEvent: event];
      }
    }
  } while (event != nil);

  return (OS_EventList)ZeroStruct();
}
