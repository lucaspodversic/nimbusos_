/* NimbusOS — mapa de memoria UEFI → formato del kernel. */
#include "bootmgr.h"
#ifdef NIMBUS_HOST
#include <string.h>
void BootFillMemoryMap(NIMBUS_BOOT_INFO* Info, void* Buf, uint32_t Size) {
    if (!Info || !Buf) return;
    memset(Buf, 0, Size);
    Info->MemoryMap = (uint64_t)(uintptr_t)Buf;
    Info->MemoryMapSize = Size;
}
#else
void BootFillMemoryMap(NIMBUS_BOOT_INFO* Info, void* Buf, uint32_t Size) {
    (void)Info; (void)Buf; (void)Size;
}
#endif
