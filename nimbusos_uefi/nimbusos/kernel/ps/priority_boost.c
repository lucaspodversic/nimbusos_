/* NimbusOS — boost temporal de prioridad. */
#include "ps.h"
void PsPriorityBoost(PETHREAD Thread, LONG Delta) {
    if (!Thread) return;
    Thread->Tcb.Priority += Delta;
    if (Thread->Tcb.Priority < 0) Thread->Tcb.Priority = 0;
    if (Thread->Tcb.Priority >= KE_PRIORITY_LEVELS)
        Thread->Tcb.Priority = KE_PRIORITY_LEVELS - 1;
}
