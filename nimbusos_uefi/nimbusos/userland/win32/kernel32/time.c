#include "../../include/win32_host.h"
#include <time.h>
#ifdef _WIN32
#else
#include <sys/time.h>
#endif
DWORD NimbusGetTickCount(void) {
#if defined(__linux__)
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return (DWORD)(ts.tv_sec * 1000u + ts.tv_nsec / 1000000u);
#else
    return (DWORD)(clock() * 1000 / CLOCKS_PER_SEC);
#endif
}
void NimbusSleep(DWORD ms) {
    struct timespec ts; ts.tv_sec = ms/1000; ts.tv_nsec = (ms%1000)*1000000L;
    nanosleep(&ts, NULL);
}
