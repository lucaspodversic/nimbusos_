/* NimbusOS — contadores de rendimiento. */
#include "etw.h"
static ULONGLONG g_Counters[64];
ULONGLONG EtwReadCounter(ULONG CounterId) {
    return CounterId < 64 ? g_Counters[CounterId] : 0;
}
void EtwIncrementCounter(ULONG CounterId) {
    if (CounterId < 64) g_Counters[CounterId]++;
}
