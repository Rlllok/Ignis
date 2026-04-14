#pragma once

#include "base/base_include.h"

#include "os/os_gfx.h"

// --AlNov: @NOTE Have to do trick with push/pop macro (not in C standard, but de facto standard in compilers)
#pragma push_macro("func")
#undef func
#include <Cocoa/Cocoa.h>
#include <QuartzCore/CAMetalLayer.h>
#pragma pop_macro("func")

typedef struct OS_MacOS OS_MacOS;
struct OS_MacOS {
  void* ns_app;
} _os_macos_state;

@interface OS_MacOS_View : NSView
@property (nonatomic, strong, readwrite) CAMetalLayer* metal_layer;
- (CAMetalLayer*)MetalLayer;
@end

typedef struct OS_MacOS_Window OS_MacOS_Window;
struct OS_MacOS_Window {
  OS_Window header;
  void* ns_window;
  void* ns_view;
};
