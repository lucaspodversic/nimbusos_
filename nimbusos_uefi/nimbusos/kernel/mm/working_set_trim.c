/* NimbusOS — recorte agresivo de working set en segundo plano. */
#include "mm.h"
#include "../host/host_compat.h"

extern ULONG MmWorkingSetGet(void);
void MmWorkingSetSet(ULONG v);

void MmWorkingSetTrim(ULONG TargetPages) {
    ULONG cur = MmWorkingSetGet();
    if (cur > TargetPages)
        MmWorkingSetSet(TargetPages);
}
