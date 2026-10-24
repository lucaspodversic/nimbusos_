/* NimbusOS — notificación de presión de memoria. */
#include "mm.h"
#include "cc.h"
#include "../host/host_compat.h"

static LONG g_PressureLevel;

void MmMemoryPressureNotify(LONG Level) {
    g_PressureLevel = Level;
    if (Level >= 2) {
        MmWorkingSetTrim(256);
        CcShrink(128);
    } else if (Level >= 1) {
        MmWorkingSetTrim(64);
    }
}
LONG MmGetPressureLevel(void) { return g_PressureLevel; }
