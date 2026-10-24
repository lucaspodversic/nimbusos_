/* NimbusOS — Deferred Procedure Calls (cola simple). */
#include "ke.h"
#include "../host/host_compat.h"

#define KE_DPC_QUEUE 64
static KDPC* g_DpcQ[KE_DPC_QUEUE];
static ULONG g_DpcHead, g_DpcTail;

void KeInitializeDpc(PKDPC Dpc, void (*fn)(void*), void* ctx) {
    if (!Dpc) return;
    nimbus_host_memset(Dpc, 0, sizeof(*Dpc));
    Dpc->DeferredRoutine = fn;
    Dpc->DeferredContext = ctx;
}

void KeInsertQueueDpc(PKDPC Dpc) {
    ULONG next;
    if (!Dpc || Dpc->Queued) return;
    next = (g_DpcTail + 1) % KE_DPC_QUEUE;
    if (next == g_DpcHead) return; /* llena */
    g_DpcQ[g_DpcTail] = Dpc;
    g_DpcTail = next;
    Dpc->Queued = 1;
}

void KeFlushDpcQueue(void) {
    while (g_DpcHead != g_DpcTail) {
        PKDPC d = g_DpcQ[g_DpcHead];
        g_DpcHead = (g_DpcHead + 1) % KE_DPC_QUEUE;
        if (d) {
            d->Queued = 0;
            if (d->DeferredRoutine) d->DeferredRoutine(d->DeferredContext);
        }
    }
}
