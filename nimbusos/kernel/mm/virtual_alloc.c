/* NimbusOS — asignación virtual (en host: malloc alineado a página). */
#include "mm.h"
#include "../host/host_compat.h"

PVOID MmAllocateVirtual(SIZE_T Bytes, ULONG Protect) {
    void* p;
    (void)Protect;
    if (Bytes == 0) return NULL;
    /* Redondeo a página */
    Bytes = (Bytes + PAGE_SIZE - 1) & ~(SIZE_T)(PAGE_SIZE - 1);
    p = nimbus_host_calloc(1, Bytes);
    return p;
}

void MmFreeVirtual(PVOID Address, SIZE_T Bytes) {
    (void)Bytes;
    nimbus_host_free(Address);
}

NTSTATUS MmProtectVirtual(PVOID Address, SIZE_T Bytes, ULONG NewProtect) {
    (void)Address; (void)Bytes; (void)NewProtect;
    return STATUS_SUCCESS; /* host: mprotect se podría usar; aquí no-op OK */
}
