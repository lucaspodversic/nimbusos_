/* NimbusOS — eventos de sincronización (notification / synchronization).
 * Modelo simplificado compatible con la semántica documentada de KeEvent:
 *  - ManualReset=1 (NotificationEvent): Set deja señalizado hasta Reset.
 *  - ManualReset=0 (SynchronizationEvent): un waiter lo consume (auto-reset).
 */
#include "ke.h"
#include "../host/host_compat.h"

void KeInitializeEvent(PKEVENT Event, LONG ManualReset, LONG InitialState) {
    if (!Event) return;
    nimbus_host_memset(Event, 0, sizeof(*Event));
    Event->ManualReset = ManualReset ? 1 : 0;
    Event->Signaled = InitialState ? 1 : 0;
}

LONG KeSetEvent(PKEVENT Event) {
    LONG prev;
    if (!Event) return 0;
    prev = Event->Signaled;
    Event->Signaled = 1;
    return prev;
}

LONG KeResetEvent(PKEVENT Event) {
    LONG prev;
    if (!Event) return 0;
    prev = Event->Signaled;
    Event->Signaled = 0;
    return prev;
}

LONG KeReadStateEvent(PKEVENT Event) {
    return Event ? Event->Signaled : 0;
}

/* Espera simple por sondeo (host). TimeoutMs=0xFFFFFFFF = infinito. */
NTSTATUS KeWaitForSingleObject(PKEVENT Event, ULONG TimeoutMs) {
    ULONG waited = 0;
    if (!Event) return STATUS_INVALID_PARAMETER;
    for (;;) {
        if (Event->Signaled) {
            if (!Event->ManualReset)
                Event->Signaled = 0; /* auto-reset */
            return STATUS_SUCCESS;
        }
        if (TimeoutMs != 0xFFFFFFFFu && waited >= TimeoutMs)
            return STATUS_UNSUCCESSFUL;
#ifdef NIMBUS_HOST
        /* 1 ms de espera aproximada */
        struct timespec ts = {0, 1000 * 1000};
        nanosleep(&ts, NULL);
#endif
        waited++;
        KeAdvanceTick(1);
        KeTimerTick(KeQueryInterruptTime());
    }
}

NTSTATUS KeWaitMultiple(PKEVENT* Events, ULONG Count, LONG WaitAll, ULONG TimeoutMs) {
    ULONG waited = 0;
    ULONG i;
    if (!Events || Count == 0) return STATUS_INVALID_PARAMETER;
    for (;;) {
        if (WaitAll) {
            LONG all = 1;
            for (i = 0; i < Count; i++) {
                if (!Events[i] || !Events[i]->Signaled) { all = 0; break; }
            }
            if (all) {
                for (i = 0; i < Count; i++)
                    if (!Events[i]->ManualReset) Events[i]->Signaled = 0;
                return STATUS_SUCCESS;
            }
        } else {
            for (i = 0; i < Count; i++) {
                if (Events[i] && Events[i]->Signaled) {
                    if (!Events[i]->ManualReset) Events[i]->Signaled = 0;
                    return STATUS_SUCCESS;
                }
            }
        }
        if (TimeoutMs != 0xFFFFFFFFu && waited >= TimeoutMs)
            return STATUS_UNSUCCESSFUL;
#ifdef NIMBUS_HOST
        struct timespec ts = {0, 1000 * 1000};
        nanosleep(&ts, NULL);
#endif
        waited++;
        KeAdvanceTick(1);
    }
}
