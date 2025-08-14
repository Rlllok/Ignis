#include "os_linux_x11_gfx.h"

#include <X11/Xlib.h>
#include <time.h>

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
	out->handle->sync_request_atom = XInternAtom(out->handle->display, "_NET_WM_SYNC_REQUEST", 0);
	out->handle->sync_request_counter_atom = XInternAtom(out->handle->display, "_NET_WM_SYNC_REQUEST_COUNTER", 0);
	out->handle->xim = XOpenIM(out->handle->display, 0, 0, 0);

	out->handle->window = XCreateWindow(
		out->handle->display, XDefaultRootWindow(out->handle->display), 0, 0,
		size.w, size.h, 0, CopyFromParent, CopyFromParent,
		CopyFromParent, 0, 0
	);

	XSelectInput(
		out->handle->display, out->handle->window,
		ExposureMask|
		StructureNotifyMask
	);

	Atom protocols[] = {
		out->handle->delete_window_atom,
		out->handle->sync_request_atom,
	};
	XSetWMProtocols(out->handle->display, out->handle->window, protocols, CountArrayElements(protocols));
	{
		XSyncValue initial_sync_value;
		XSyncIntToValue(&initial_sync_value, 0);
		out->handle->sync_counter_xid = XSyncCreateCounter(out->handle->display, initial_sync_value);
	}
	XChangeProperty(
			out->handle->display, out->handle->window,
			out->handle->sync_request_counter_atom, XA_CARDINAL,
			32, PropModeReplace, (U8*)&out->handle->sync_counter_xid, 1);

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
	while(XPending(window->handle->display))
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
				else if ((Atom)x_event.xclient.data.l[0] == window->handle->sync_request_atom)
				{
					window->handle->sync_counter_value = 0;
					window->handle->sync_counter_value |= x_event.xclient.data.l[2];
					window->handle->sync_counter_value |= (x_event.xclient.data.l[3] << 32);

					XSyncValue sync_value;
					XSyncIntToValue(&sync_value, window->handle->sync_counter_value);
					XSyncSetCounter(window->handle->display, window->handle->sync_counter_xid, sync_value);
					LOG_DEBUG("Sync updated\n");
				}
			} break;

			case ConfigureNotify:
			{
				Vec2U32 event_window_size = MakeVec2U32((U32)x_event.xconfigure.width, (U32)x_event.xconfigure.height);
				if ((event_window_size.w != window->size.w) || (event_window_size.h != window->size.h))
				{
					OS_Event event = {
						.type = OS_EVENT_TYPE_RESIZE,
						.window_size = event_window_size,
					};
					PushListOS_Event(&_os_state.event_list, event);
				}
			} break;
		}
	}
	return _os_state.event_list;
}

func F32 OS_CurrentTimeSeconds(void)
{
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC_RAW, &now);
	return now.tv_sec + now.tv_nsec*0.000000001;
}

func Vec2F32 OS_MousePosition(OS_Window window)
{
	return MakeVec2F32(0.0f, 0.0f);
}
