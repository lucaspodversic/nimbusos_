/* NimbusOS — hilos. */
#include "ps.h"
#include "../host/host_compat.h"

NTSTATUS PsCreateThread(PEPROCESS Process, PETHREAD* Out, void (*Start)(void*), void* Ctx) {
    PETHREAD t;
    if (!Out || !Start) return STATUS_INVALID_PARAMETER;
    t = (PETHREAD)nimbus_host_calloc(1, sizeof(ETHREAD));
    if (!t) return STATUS_NO_MEMORY;
    t->Process = Process;
    t->Tcb.Priority = Process ? Process->Pcb.BasePriority : 8;
    t->Tcb.Process = Process ? &Process->Pcb : NULL;
    KeStartThread(&t->Tcb, Start, Ctx);
    if (Process) Process->Pcb.ThreadCount++;
    *Out = t;
    return STATUS_SUCCESS;
}
