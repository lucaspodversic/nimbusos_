/* NimbusOS — carga del kernel desde ESP. */
#include "bootmgr.h"
#ifdef NIMBUS_HOST
#include <stdlib.h>
int BootLoadKernel(NIMBUS_BOOT_INFO* Info) {
    /* Simula kernel de 1 MB cargado. */
    void* k = calloc(1, 1024 * 1024);
    if (!k) return -1;
    Info->KernelPhysical = (uint64_t)(uintptr_t)k;
    Info->KernelSize = 1024 * 1024;
    return 0;
}
#else
int BootLoadKernel(NIMBUS_BOOT_INFO* Info) { (void)Info; return -1; }
#endif
