/* NimbusOS — write-behind. */
#include "cc.h"
#include "../host/host_compat.h"
extern CC_CACHE_MAP* CcGetMaps(void);
NTSTATUS CcFlushWriteBehind(void) {
    ULONG i; CC_CACHE_MAP* m = CcGetMaps();
    for (i = 0; i < CC_MAX_MAPS; i++)
        if (m[i].InUse && m[i].Dirty) m[i].Dirty = 0;
    return STATUS_SUCCESS;
}
