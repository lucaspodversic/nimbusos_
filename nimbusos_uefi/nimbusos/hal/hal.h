/* NimbusOS — Hardware Abstraction Layer. */
#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
int  HalInit(void);
void HalHalt(void);
uint64_t HalReadTsc(void);
void HalInitApic(void);
void HalInitIoApic(void);
void HalInitHpet(void);
void HalInitSmp(void);
void HalLoadIdtGdt(void);
uint32_t HalCpuidFeature(uint32_t leaf, uint32_t* ebx, uint32_t* ecx, uint32_t* edx);
#ifdef __cplusplus
}
#endif
