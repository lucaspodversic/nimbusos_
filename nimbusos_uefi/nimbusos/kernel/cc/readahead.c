/* NimbusOS — readahead adaptativo con presupuesto. */
#include "cc.h"
void CcReadAhead(ULONG64 FileId, SIZE_T Offset, SIZE_T Length) {
    PVOID p = NULL;
    CcMapData(FileId, Offset + Length, &p);
    (void)p;
}
