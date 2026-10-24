/* NimbusOS — Asynchronous Procedure Calls (entrega en contexto de hilo). */
#include "ke.h"
#include "../host/host_compat.h"

#define KE_APC_QUEUE 64
static KAPC* g_ApcQ[KE_APC_QUEUE];
static ULONG g_ApcN;

void KeInitializeApc(PKAPC Apc, void (*fn)(void*), void* ctx) {
    if (!Apc) return;
    nimbus_host_memset(Apc, 0, sizeof(*Apc));
    Apc->KernelRoutine = fn;
    Apc->NormalContext = ctx;
}

void KeInsertQueueApc(PKAPC Apc) {
    if (!Apc || Apc->Inserted) return;
    if (g_ApcN >= KE_APC_QUEUE) return;
    g_ApcQ[g_ApcN++] = Apc;
    Apc->Inserted = 1;
}

void KeDeliverApcs(void) {
    ULONG i;
    for (i = 0; i < g_ApcN; i++) {
        PKAPC a = g_ApcQ[i];
        if (a && a->KernelRoutine) a->KernelRoutine(a->NormalContext);
        if (a) a->Inserted = 0;
    }
    g_ApcN = 0;
}
