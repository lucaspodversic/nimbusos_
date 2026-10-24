/* NimbusOS HAL — hal/tsc.c */
#include "hal.h"
#ifdef NIMBUS_HOST
#include <time.h>
uint64_t HalReadTsc(void) {
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}
#else
uint64_t HalReadTsc(void) {
    uint32_t lo, hi; __asm__ __volatile__("rdtsc":"=a"(lo),"=d"(hi));
    return ((uint64_t)hi<<32)|lo;
}
#endif
