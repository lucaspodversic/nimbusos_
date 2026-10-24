/* NimbusOS — Boot Manager UEFI (clean-room). */
#pragma once
#include <stdint.h>
#ifdef NIMBUS_HOST
#include <stddef.h>
#endif

typedef struct _NIMBUS_BOOT_INFO {
    uint64_t KernelPhysical;
    uint64_t KernelSize;
    uint64_t MemoryMap;
    uint32_t MemoryMapSize;
    uint32_t AbSlot; /* 0=A, 1=B */
    uint32_t SecureBoot;
} NIMBUS_BOOT_INFO;

int BootMgrMain(NIMBUS_BOOT_INFO* Info);
int BootSelectAbSlot(void);
int BootVerifySecure(const void* Image, uint64_t Size);
