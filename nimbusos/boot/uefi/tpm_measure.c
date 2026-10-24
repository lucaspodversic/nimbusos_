/* NimbusOS — medición TPM PCR (simulada en host). */
#include "bootmgr.h"
int BootMeasureTpm(const void* Image, uint64_t Size) {
    (void)Image; (void)Size;
    return 0; /* OK */
}
