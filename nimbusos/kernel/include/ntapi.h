/* NimbusOS — declaraciones de syscalls / API interna del núcleo (clean-room). */
#pragma once
#include "ntdef.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Inicialización global del núcleo (host o bare-metal). */
NTSTATUS KiSystemStartup(void);
void     KiSystemShutdown(void);

/* Contador de ticks simulado (host) o HAL (bare-metal). */
ULONG64  KeQueryInterruptTime(void);
void     KeAdvanceTick(ULONG64 delta);

#ifdef __cplusplus
}
#endif
