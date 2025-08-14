#pragma once

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/sync.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>

typedef struct OS_WindowHandle OS_WindowHandle;
struct OS_WindowHandle
{
	Display* display;
	Window window;
	XIC xic;
	XID sync_counter_xid;
	U64 sync_counter_value;

	Atom delete_window_atom;
	Atom sync_request_atom;
	Atom sync_request_counter_atom;

	XIM xim;
};
