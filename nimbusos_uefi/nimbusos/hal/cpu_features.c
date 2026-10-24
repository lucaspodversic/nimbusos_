/* NimbusOS HAL — cpu features */
#include "hal.h"
uint32_t HalCpuidFeature(uint32_t leaf, uint32_t* ebx, uint32_t* ecx, uint32_t* edx) {
#ifdef NIMBUS_HOST
    (void)leaf;
    if (ebx) *ebx = 0;
    if (ecx) *ecx = 0;
    if (edx) *edx = 0;
    return 0;
#else
    uint32_t eax = 0;
    __asm__ __volatile__("cpuid":"=a"(eax),"=b"(*ebx),"=c"(*ecx),"=d"(*edx):"a"(leaf));
    return eax;
#endif
}
