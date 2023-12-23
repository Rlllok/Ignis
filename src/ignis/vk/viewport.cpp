#include "viewport.h"

#include "ignis/window/win_window.h"

Viewport::Viewport(const Device& device)
    : device(device)
{
    window = new Window("Test", 1280, 720);
}

Viewport::~Viewport()
{
    delete window;
}

