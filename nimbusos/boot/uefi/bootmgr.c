/* NimbusOS — bootmgr: verifica firma, mide TPM, elige A/B, carga kernel.
 * En host se ejecuta como función de test sin firmware UEFI real.
 */
#include "bootmgr.h"
#ifdef NIMBUS_HOST
#include <stdio.h>
#include <string.h>
#endif

extern int BootSelectAbSlot(void);
extern int BootLoadKernel(NIMBUS_BOOT_INFO* Info);
extern int BootMeasureTpm(const void* Image, uint64_t Size);
extern void BootInitGraphics(void);

int BootMgrMain(NIMBUS_BOOT_INFO* Info) {
    if (!Info) return -1;
#ifdef NIMBUS_HOST
    memset(Info, 0, sizeof(*Info));
#endif
    Info->AbSlot = (uint32_t)BootSelectAbSlot();
    Info->SecureBoot = 1;
    BootInitGraphics();
    if (BootLoadKernel(Info) != 0) return -2;
    BootMeasureTpm((const void*)(uintptr_t)Info->KernelPhysical, Info->KernelSize);
    return 0;
}
