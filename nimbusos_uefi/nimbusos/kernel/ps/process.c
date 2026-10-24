/* NimbusOS — procesos. */
#include "ps.h"
#include "../host/host_compat.h"

static ULONG g_NextPid = 1;

NTSTATUS PsInitialize(void) { g_NextPid = 1; return STATUS_SUCCESS; }
void PsShutdown(void) {}

NTSTATUS PsCreateProcess(PEPROCESS* Out, const char* ImageName, PTOKEN Token) {
    PEPROCESS p;
    if (!Out) return STATUS_INVALID_PARAMETER;
    p = (PEPROCESS)nimbus_host_calloc(1, sizeof(EPROCESS));
    if (!p) return STATUS_NO_MEMORY;
    p->Pcb.ProcessId = g_NextPid++;
    if (ImageName) nimbus_host_strncpy(p->Pcb.ImageName, ImageName, sizeof(p->Pcb.ImageName)-1);
    p->Pcb.BasePriority = 8;
    ObInitHandleTable(&p->HandleTable);
    p->Token = Token;
    if (Token) SeReferenceToken(Token);
    *Out = p;
    return STATUS_SUCCESS;
}

NTSTATUS PsTerminateProcess(PEPROCESS Process) {
    if (!Process) return STATUS_INVALID_PARAMETER;
    if (Process->Token) SeDereferenceToken(Process->Token);
    if (Process->SectionBase) nimbus_host_free(Process->SectionBase);
    nimbus_host_free(Process);
    return STATUS_SUCCESS;
}
