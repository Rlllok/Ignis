#include "os_linux_x11_gfx.h"

#include <X11/Xlib.h>

func void OS_Init(U64 arena_size)
{
	_os_state.arena = AllocateArena(arena_size);
}

func void OS_CreateWindow(Str8 title, Vec2U32 size, OS_Window* out)
{
	out->handle = (OS_WindowHandle*)OS_AllocateMemory(sizeof(OS_WindowHandle));
	out->size = size;

	out->handle->display = XOpenDisplay(0);
	if (!out->handle->display)
	{
		return;
	}

	out->handle->delete_window_atom = XInternAtom(out->handle->display, "WM_DELETE_WINDOW", 0);
	out->handle->xim = XOpenIM(out->handle->display, 0, 0, 0);

	out->handle->window = XCreateWindow(
		out->handle->display, XDefaultRootWindow(out->handle->display), 0, 0,
		size.w, size.h, 0, CopyFromParent, CopyFromParent,
		CopyFromParent, 0, 0
	);

	XSelectInput(
		out->handle->display, out->handle->window,
		ExposureMask|
		PointerMotionMask|
		ButtonPressMask|
		ButtonReleaseMask|
		KeyPressMask|
		KeyReleaseMask|
		FocusChangeMask
	);

	Atom protocols[] = {
		out->handle->delete_window_atom,
	};
	XSetWMProtocols(out->handle->display, out->handle->window, protocols, CountArrayElements(protocols));

	out->handle->xic = XCreateIC(
		out->handle->xim,
		XNInputStyle,
		XIMPreeditNothing|XIMStatusNothing,
		XNClientWindow, out->handle->window,
		XNFocusWindow, out->handle->window,
		NULL
	);
}

func void OS_DestroyWindow(OS_Window* window)
{
	XDestroyWindow(window->handle->display, window->handle->window);
	XCloseDisplay(window->handle->display);
}

func void OS_ShowWindow(OS_Window* window)
{
	XMapWindow(window->handle->display, window->handle->window);
}

func void OS_LockCursor(OS_Window* window)
{
	// @TODO
}

func void OS_UnlockCursor(OS_Window* window)
{
	// @TODO
}

func ListOS_Event OS_GetEventList(Arena* arena, OS_Window* window)
{
	_os_state.event_list = CreateListOS_Event(arena);
	while(XPending(window->handle->display) > 0)
	{
		XEvent x_event = {0};
		XNextEvent(window->handle->display, &x_event);

		switch (x_event.type)
		{
			default: {}break;

			case ClientMessage:
			{
				if((Atom)x_event.xclient.data.l[0] == window->handle->delete_window_atom)
				{
					OS_Event event = {
						.type = OS_EVENT_TYPE_EXIT,
					};
					PushListOS_Event(&_os_state.event_list, event);
				}
			}
		}
	}
	return _os_state.event_list;
}

func F32 OS_CurrentTimeSeconds(void)
{
	// --AlNov: @TODO
	return 0;
}

func Vec2F32 OS_MousePosition(OS_Window window)
{
	return MakeVec2F32(0.0f, 0.0f);
}
