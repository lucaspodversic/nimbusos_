/* NimbusOS — Cache Manager. */
#pragma once
#include "ntdef.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CC_MAX_MAPS 256

typedef struct _CC_CACHE_MAP {
    ULONG64 FileId;
    PVOID   Base;
    SIZE_T  Length;
    LONG    Dirty;
    ULONG   Hits;
    LONG    InUse;
} CC_CACHE_MAP, *PCC_CACHE_MAP;

NTSTATUS CcInitialize(void);
void     CcShutdown(void);
NTSTATUS CcMapData(ULONG64 FileId, SIZE_T Length, PVOID* Out);
NTSTATUS CcPinData(ULONG64 FileId, SIZE_T Offset, SIZE_T Length, PVOID* Out);
void     CcSetDirty(ULONG64 FileId);
void     CcReadAhead(ULONG64 FileId, SIZE_T Offset, SIZE_T Length);
NTSTATUS CcFlushWriteBehind(void);
ULONG    CcShrink(ULONG TargetPages);
ULONG    CcMappedCount(void);

#ifdef __cplusplus
}
#endif
