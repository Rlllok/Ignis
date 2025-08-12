#pragma once

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>

typedef struct OS_WindowHandle OS_WindowHandle;
struct OS_WindowHandle
{
	Display* display;
	Window window;
	XIC xic;

	Atom delete_window_atom;

	XIM xim;
};
