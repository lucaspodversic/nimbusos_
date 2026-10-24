/* NimbusOS — Cache Manager: mapas de archivo en memoria. */
#include "cc.h"
#include "../host/host_compat.h"

static CC_CACHE_MAP g_Maps[CC_MAX_MAPS];

NTSTATUS CcInitialize(void) {
    nimbus_host_memset(g_Maps, 0, sizeof(g_Maps));
    return STATUS_SUCCESS;
}
void CcShutdown(void) {
    ULONG i;
    for (i = 0; i < CC_MAX_MAPS; i++)
        if (g_Maps[i].InUse) { nimbus_host_free(g_Maps[i].Base); g_Maps[i].InUse = 0; }
}

NTSTATUS CcMapData(ULONG64 FileId, SIZE_T Length, PVOID* Out) {
    ULONG i;
    for (i = 0; i < CC_MAX_MAPS; i++) {
        if (g_Maps[i].InUse && g_Maps[i].FileId == FileId) {
            g_Maps[i].Hits++;
            if (Out) *Out = g_Maps[i].Base;
            return STATUS_SUCCESS;
        }
    }
    for (i = 0; i < CC_MAX_MAPS; i++) {
        if (g_Maps[i].InUse) continue;
        g_Maps[i].Base = nimbus_host_calloc(1, Length ? Length : PAGE_SIZE);
        if (!g_Maps[i].Base) return STATUS_NO_MEMORY;
        g_Maps[i].FileId = FileId;
        g_Maps[i].Length = Length;
        g_Maps[i].InUse = 1;
        if (Out) *Out = g_Maps[i].Base;
        return STATUS_SUCCESS;
    }
    return STATUS_INSUFFICIENT_RESOURCES;
}

NTSTATUS CcPinData(ULONG64 FileId, SIZE_T Offset, SIZE_T Length, PVOID* Out) {
    UCHAR* base = NULL;
    NTSTATUS st = CcMapData(FileId, Offset + Length, (PVOID*)&base);
    if (st != STATUS_SUCCESS) return st;
    if (Out) *Out = base + Offset;
    return STATUS_SUCCESS;
}

void CcSetDirty(ULONG64 FileId) {
    ULONG i;
    for (i = 0; i < CC_MAX_MAPS; i++)
        if (g_Maps[i].InUse && g_Maps[i].FileId == FileId) g_Maps[i].Dirty = 1;
}

ULONG CcMappedCount(void) {
    ULONG i, n = 0;
    for (i = 0; i < CC_MAX_MAPS; i++) if (g_Maps[i].InUse) n++;
    return n;
}

CC_CACHE_MAP* CcGetMaps(void) { return g_Maps; }
