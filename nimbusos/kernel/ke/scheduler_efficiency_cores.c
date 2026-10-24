/* NimbusOS — preferencia de núcleos eficientes (E-cores) vs rendimiento (P-cores).
 * En host solo anotamos la preferencia en el hilo; el HAL SMP la usaría.
 */
#include "ke.h"

void KeSetEfficiencyPreference(PKTHREAD t, LONG PreferEfficient) {
    if (!t) return;
    /* Codificamos la preferencia en AffinityMask bit 31 como hint. */
    if (PreferEfficient)
        t->AffinityMask |= 0x80000000u;
    else
        t->AffinityMask &= ~0x80000000u;
}
