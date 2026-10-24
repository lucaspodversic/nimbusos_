/* NimbusOS — vistas compartidas ALPC. */
#include "alpc.h"
#include "mm.h"
NTSTATUS AlpcMapSharedView(PALPC_PORT Port, SIZE_T Size, PVOID* Out) {
    (void)Port;
    *Out = MmAllocateVirtual(Size, PAGE_READWRITE);
    return *Out ? STATUS_SUCCESS : STATUS_NO_MEMORY;
}
