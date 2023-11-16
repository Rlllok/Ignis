#include "win_timer.h"

WinTimer::WinTimer()
{
    QueryPerformanceFrequency(&qpFrequency);
    QueryPerformanceCounter(&qpCounter);

    qpMaxDelta = qpFrequency.QuadPart / 10; 
}

double WinTimer::getTimeSeconds()
{
    LARGE_INTEGER value{0};
    QueryPerformanceCounter(&value);

    return (double) value.QuadPart / qpFrequency.QuadPart;
}
