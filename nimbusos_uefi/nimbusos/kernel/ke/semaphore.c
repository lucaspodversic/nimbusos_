/* NimbusOS — semáforo contador con tope. */
#include "ke.h"
#include "../host/host_compat.h"

void KeInitializeSemaphore(PKSEMAPHORE Sem, LONG Count, LONG Limit) {
    if (!Sem) return;
    nimbus_host_memset(Sem, 0, sizeof(*Sem));
    Sem->Count = Count;
    Sem->Limit = Limit > 0 ? Limit : 0x7FFFFFFF;
    KeInitializeEvent(&Sem->Available, 0, Count > 0 ? 1 : 0);
}

NTSTATUS KeWaitForSemaphore(PKSEMAPHORE Sem, ULONG TimeoutMs) {
    if (!Sem) return STATUS_INVALID_PARAMETER;
    for (;;) {
        LONG c = Sem->Count;
        if (c > 0 && __sync_bool_compare_and_swap(&Sem->Count, c, c - 1)) {
            if (Sem->Count == 0) KeResetEvent(&Sem->Available);
            return STATUS_SUCCESS;
        }
        if (KeWaitForSingleObject(&Sem->Available, TimeoutMs) != STATUS_SUCCESS)
            return STATUS_UNSUCCESSFUL;
    }
}

LONG KeReleaseSemaphore(PKSEMAPHORE Sem, LONG Adjustment) {
    LONG prev;
    if (!Sem || Adjustment <= 0) return 0;
    prev = Sem->Count;
    if (prev + Adjustment > Sem->Limit)
        Sem->Count = Sem->Limit;
    else
        Sem->Count = prev + Adjustment;
    KeSetEvent(&Sem->Available);
    return prev;
}
