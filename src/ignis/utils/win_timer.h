#pragma once

#include <windows.h>

class WinTimer
{
public:
    WinTimer();

public:
    void    Tick() {}
    double  getTimeSeconds();

// Simple getters
public:
    UINT32 getFramesPerSecond() { return framesPerSecond; }

private:
    LARGE_INTEGER qpFrequency{0};
    LARGE_INTEGER qpCounter{0};

    UINT64 qpMaxDelta{0};

    UINT32 framesPerSecond{0};
};