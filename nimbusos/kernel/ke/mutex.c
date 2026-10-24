/* NimbusOS — mutex recursivo con dueño por ThreadId. */
#include "ke.h"
#include "../host/host_compat.h"

void KeInitializeMutex(PKMUTEX Mutex) {
    if (!Mutex) return;
    nimbus_host_memset(Mutex, 0, sizeof(*Mutex));
    KeInitializeEvent(&Mutex->Available, 0, 1); /* inicialmente libre */
}

NTSTATUS KeAcquireMutex(PKMUTEX Mutex, ULONG TimeoutMs) {
    PKTHREAD cur;
    if (!Mutex) return STATUS_INVALID_PARAMETER;
    cur = KeGetCurrentThread();
    if (cur && Mutex->OwnerThreadId == (LONG)cur->ThreadId) {
        Mutex->RecursionCount++;
        return STATUS_SUCCESS;
    }
    /* Espera a que Available esté señalizado (libre). */
    if (KeWaitForSingleObject(&Mutex->Available, TimeoutMs) != STATUS_SUCCESS)
        return STATUS_UNSUCCESSFUL;
    Mutex->OwnerThreadId = cur ? (LONG)cur->ThreadId : 1;
    Mutex->RecursionCount = 1;
    return STATUS_SUCCESS;
}

void KeReleaseMutex(PKMUTEX Mutex) {
    if (!Mutex) return;
    if (Mutex->RecursionCount > 1) {
        Mutex->RecursionCount--;
        return;
    }
    Mutex->RecursionCount = 0;
    Mutex->OwnerThreadId = 0;
    KeSetEvent(&Mutex->Available);
}
