/* Critical sections RTL (host: pthread-free spin simple). */
#include "../../include/win32_host.h"
#include <stdatomic.h>
typedef struct { atomic_int lock; } RTL_CRITICAL_SECTION;
void RtlInitializeCriticalSection(RTL_CRITICAL_SECTION* cs) { atomic_store(&cs->lock, 0); }
void RtlEnterCriticalSection(RTL_CRITICAL_SECTION* cs) {
    while (atomic_exchange(&cs->lock, 1)) { /* spin */ }
}
void RtlLeaveCriticalSection(RTL_CRITICAL_SECTION* cs) { atomic_store(&cs->lock, 0); }
