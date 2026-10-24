/* NimbusOS — pool del núcleo con cargo a presupuesto. */
#include "mm.h"
#include "../host/host_compat.h"

PVOID MmAllocatePool(SIZE_T Bytes, NIMBUS_COMPONENT_ID Comp) {
    PVOID p;
    if (MmBudgetTryCharge(Comp, Bytes) != STATUS_SUCCESS)
        return NULL;
    p = nimbus_host_calloc(1, Bytes);
    if (!p) MmBudgetRelease(Comp, Bytes);
    return p;
}

void MmFreePool(PVOID P, SIZE_T Bytes, NIMBUS_COMPONENT_ID Comp) {
    nimbus_host_free(P);
    MmBudgetRelease(Comp, Bytes);
}
