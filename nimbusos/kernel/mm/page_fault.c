/* NimbusOS — manejador de page fault (COW / demand zero). */
#include "mm.h"
#include "../host/host_compat.h"

void MmHandlePageFault(PVOID Address, LONG Write) {
    if (Write)
        MmCowFault(Address);
    /* Demand-zero: en host las páginas ya vienen de calloc. */
    (void)Address;
}
