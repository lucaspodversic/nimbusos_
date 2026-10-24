/* NimbusOS — DMA helpers. */
#include "io.h"
#include "../host/host_compat.h"
NTSTATUS IoDmaAllocate(SIZE_T Bytes, PVOID* Out) {
    if (!Out) return STATUS_INVALID_PARAMETER;
    *Out = nimbus_host_calloc(1, Bytes);
    return *Out ? STATUS_SUCCESS : STATUS_NO_MEMORY;
}
void IoDmaFree(PVOID P, SIZE_T Bytes) { (void)Bytes; nimbus_host_free(P); }
