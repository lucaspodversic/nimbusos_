/* NimbusOS — asignador de marcos de página físicos (first-fit). */
#include "mm.h"
#include "../host/host_compat.h"

extern PMMPFN MmGetPfnDb(void);
extern ULONG  MmGetPageCount(void);
extern void   MmSetFreeCount(ULONG c);

NTSTATUS MmAllocatePhysicalPages(ULONG PageCount, ULONG* OutFirstPfn) {
    ULONG i, j, n;
    PMMPFN db;
    if (!OutFirstPfn || PageCount == 0) return STATUS_INVALID_PARAMETER;
    db = MmGetPfnDb();
    n = MmGetPageCount();
    if (!db) return STATUS_NO_MEMORY;
    for (i = 0; i + PageCount <= n; i++) {
        LONG ok = 1;
        for (j = 0; j < PageCount; j++) {
            if ((db[i+j].Flags & 0x1) == 0) { ok = 0; break; }
        }
        if (!ok) continue;
        for (j = 0; j < PageCount; j++) {
            db[i+j].Flags &= ~0x1u;
            db[i+j].RefCount = 1;
        }
        MmSetFreeCount(MmGetFreePageCount() - PageCount);
        *OutFirstPfn = i;
        return STATUS_SUCCESS;
    }
    return STATUS_NO_MEMORY;
}

void MmFreePhysicalPages(ULONG FirstPfn, ULONG PageCount) {
    ULONG j;
    PMMPFN db = MmGetPfnDb();
    ULONG n = MmGetPageCount();
    if (!db || FirstPfn >= n) return;
    if (FirstPfn + PageCount > n) PageCount = n - FirstPfn;
    for (j = 0; j < PageCount; j++) {
        db[FirstPfn+j].Flags |= 0x1;
        db[FirstPfn+j].RefCount = 0;
        db[FirstPfn+j].ShareCount = 0;
        db[FirstPfn+j].Flags &= ~(0x4u | 0x8u); /* clear compressed/shared */
    }
    MmSetFreeCount(MmGetFreePageCount() + PageCount);
}
