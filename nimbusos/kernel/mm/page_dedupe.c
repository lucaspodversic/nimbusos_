/* NimbusOS — deduplicación de páginas (KSM-style, clean-room).
 * Hash de 64 bits por página; páginas idénticas comparten marco (ShareCount++).
 */
#include "mm.h"
#include "../host/host_compat.h"

extern UCHAR* MmGetPhysArena(void);
extern PMMPFN MmGetPfnDb(void);
extern ULONG  MmGetPageCount(void);

typedef struct _DEDUPE_ENTRY {
    ULONG64 Hash;
    ULONG   Pfn;
    LONG    InUse;
} DEDUPE_ENTRY;

#define DEDUPE_BUCKETS 1024
static DEDUPE_ENTRY g_Tab[DEDUPE_BUCKETS];
static ULONG g_SharedPages;

ULONG64 MmHashPage(const void* Page) {
    /* FNV-1a 64-bit sobre 4096 bytes. */
    const UCHAR* p = (const UCHAR*)Page;
    ULONG64 h = 14695981039346656037ULL;
    SIZE_T i;
    for (i = 0; i < PAGE_SIZE; i++) {
        h ^= p[i];
        h *= 1099511628211ULL;
    }
    return h;
}

NTSTATUS MmDedupeInit(void) {
    nimbus_host_memset(g_Tab, 0, sizeof(g_Tab));
    g_SharedPages = 0;
    return STATUS_SUCCESS;
}

ULONG MmDedupeSharedPages(void) { return g_SharedPages; }

NTSTATUS MmDedupeScan(void) {
    ULONG i, n = MmGetPageCount();
    UCHAR* arena = MmGetPhysArena();
    PMMPFN db = MmGetPfnDb();
    if (!arena || !db) return STATUS_NO_MEMORY;
    g_SharedPages = 0;
    nimbus_host_memset(g_Tab, 0, sizeof(g_Tab));
    for (i = 0; i < n; i++) {
        ULONG64 h;
        ULONG b;
        if (db[i].Flags & 0x1) continue; /* free */
        if (db[i].Flags & 0x4) continue; /* compressed */
        h = MmHashPage(arena + (SIZE_T)i * PAGE_SIZE);
        db[i].ContentHash = h;
        b = (ULONG)(h % DEDUPE_BUCKETS);
        if (g_Tab[b].InUse && g_Tab[b].Hash == h) {
            /* Coincide: marcar ambas como shared (simplificado: solo contamos). */
            db[i].Flags |= 0x8;
            db[i].ShareCount++;
            db[g_Tab[b].Pfn].Flags |= 0x8;
            db[g_Tab[b].Pfn].ShareCount++;
            g_SharedPages++;
        } else {
            g_Tab[b].InUse = 1;
            g_Tab[b].Hash = h;
            g_Tab[b].Pfn = i;
        }
    }
    return STATUS_SUCCESS;
}
