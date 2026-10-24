/* NimbusOS — almacén zram: páginas físicas comprimidas en slots. */
#include "mm.h"
#include "../host/host_compat.h"

extern UCHAR* MmGetPhysArena(void);
extern PMMPFN MmGetPfnDb(void);

typedef struct _ZRAM_SLOT {
    LONG   InUse;
    ULONG  Pfn;
    USHORT CompSize;
    UCHAR  Data[2048]; /* suficiente para páginas muy repetitivas */
} ZRAM_SLOT;

static ZRAM_SLOT g_Slots[MM_ZRAM_SLOT_MAX];
static ULONG     g_CompCount;

NTSTATUS MmZramInit(void) {
    nimbus_host_memset(g_Slots, 0, sizeof(g_Slots));
    g_CompCount = 0;
    return STATUS_SUCCESS;
}

ULONG MmZramCompressedCount(void) { return g_CompCount; }

NTSTATUS MmZramCompressPage(ULONG Pfn, const void* PageData) {
    ULONG i;
    SIZE_T csz;
    UCHAR tmp[PAGE_SIZE];
    const void* src = PageData;
    PMMPFN db = MmGetPfnDb();
    UCHAR* arena = MmGetPhysArena();

    if (!db) return STATUS_NO_MEMORY;
    if (!src && arena) src = arena + (SIZE_T)Pfn * PAGE_SIZE;
    if (!src) return STATUS_INVALID_PARAMETER;

    csz = MmLz4Compress(src, PAGE_SIZE, tmp, sizeof(tmp));
    if (csz == 0 || csz >= sizeof(g_Slots[0].Data)) {
        /* No comprimió bien: igual guardamos prefijo truncado fallando */
        return STATUS_UNSUCCESSFUL;
    }
    for (i = 0; i < MM_ZRAM_SLOT_MAX; i++) {
        if (g_Slots[i].InUse) continue;
        g_Slots[i].InUse = 1;
        g_Slots[i].Pfn = Pfn;
        g_Slots[i].CompSize = (USHORT)csz;
        nimbus_host_memcpy(g_Slots[i].Data, tmp, csz);
        db[Pfn].Flags |= 0x4; /* compressed */
        db[Pfn].ZramIndex = i;
        g_CompCount++;
        return STATUS_SUCCESS;
    }
    return STATUS_INSUFFICIENT_RESOURCES;
}

NTSTATUS MmZramDecompressPage(ULONG Pfn, void* OutPage) {
    PMMPFN db = MmGetPfnDb();
    ULONG idx;
    SIZE_T out;
    if (!db || !OutPage) return STATUS_INVALID_PARAMETER;
    if ((db[Pfn].Flags & 0x4) == 0) return STATUS_NOT_FOUND;
    idx = db[Pfn].ZramIndex;
    if (idx >= MM_ZRAM_SLOT_MAX || !g_Slots[idx].InUse)
        return STATUS_NOT_FOUND;
    out = MmLz4Decompress(g_Slots[idx].Data, g_Slots[idx].CompSize, OutPage, PAGE_SIZE);
    if (out != PAGE_SIZE) return STATUS_UNSUCCESSFUL;
    /* liberar slot */
    g_Slots[idx].InUse = 0;
    db[Pfn].Flags &= ~0x4u;
    g_CompCount--;
    return STATUS_SUCCESS;
}
