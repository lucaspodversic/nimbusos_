/* NimbusOS — inicialización HAL. */
#include "hal.h"
#ifdef NIMBUS_HOST
#include <stdio.h>
#endif
int HalInit(void) {
    HalLoadIdtGdt();
    HalInitApic();
    HalInitIoApic();
    HalInitHpet();
    HalInitSmp();
#ifdef NIMBUS_HOST
    printf("NimbusOS HAL init (host)\n");
#endif
    return 0;
}
void HalHalt(void) {
#ifdef NIMBUS_HOST
    return;
#else
    for(;;){ __asm__ __volatile__("hlt"); }
#endif
}
