/* NimbusOS — shrink del cache bajo presión. */
#include "cc.h"
#include "../host/host_compat.h"
extern CC_CACHE_MAP* CcGetMaps(void);
ULONG CcShrink(ULONG TargetPages) {
    ULONG i, freed = 0; CC_CACHE_MAP* m = CcGetMaps();
    for (i = 0; i < CC_MAX_MAPS && freed < TargetPages; i++) {
        if (!m[i].InUse || m[i].Dirty) continue;
        nimbus_host_free(m[i].Base);
        nimbus_host_memset(&m[i], 0, sizeof(m[i]));
        freed++;
    }
    return freed;
}
