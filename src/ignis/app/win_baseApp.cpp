#include "win_baseApp.h"

#include <iostream>

#include "ignis/window/win_window.h"

IBaseApp::~IBaseApp()
{
    delete window;

    delete commandPool;
    delete swapchain;
    delete device;
    delete instance;
}

void IBaseApp::initWindow(const char* name, int width, int height)
{
    window = new Window(name, width, height);
}

void IBaseApp::showWindow()
{
    window->show();
}
