/* NimbusOS — afinidad de CPU. */
#include "ps.h"
void PsSetAffinity(PETHREAD Thread, ULONG Mask) {
    if (Thread) {
        Thread->Tcb.AffinityMask = Mask;
        Thread->Process ? (Thread->Process->Affinity = Mask) : 0;
    }
}
