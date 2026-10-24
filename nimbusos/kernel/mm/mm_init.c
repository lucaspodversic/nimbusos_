/* NimbusOS — inicialización del Memory Manager. */
#include "mm.h"
#include "../host/host_compat.h"

/* Arena física simulada (host): un gran bloque malloc. */
static UCHAR* g_PhysArena = NULL;
static SIZE_T g_PhysBytes = 0;
static ULONG  g_PageCount = 0;
static MMPFN* g_PfnDb = NULL;
static ULONG  g_FreeCount = 0;
static LONG   g_MmInit = 0;

UCHAR* MmGetPhysArena(void) { return g_PhysArena; }
ULONG  MmGetPageCount(void) { return g_PageCount; }
PMMPFN MmGetPfnDb(void) { return g_PfnDb; }

NTSTATUS MmInitialize(SIZE_T PhysicalBytes) {
    ULONG i;
    if (g_MmInit) return STATUS_SUCCESS;
    if (PhysicalBytes < PAGE_SIZE * 16) PhysicalBytes = PAGE_SIZE * 16;
    if (PhysicalBytes > (SIZE_T)MM_MAX_PHYSICAL_PAGES * PAGE_SIZE)
        PhysicalBytes = (SIZE_T)MM_MAX_PHYSICAL_PAGES * PAGE_SIZE;
    g_PhysBytes = PhysicalBytes;
    g_PageCount = (ULONG)(PhysicalBytes / PAGE_SIZE);
    g_PhysArena = (UCHAR*)nimbus_host_calloc(1, g_PhysBytes);
    g_PfnDb = (PMMPFN)nimbus_host_calloc(g_PageCount, sizeof(MMPFN));
    if (!g_PhysArena || !g_PfnDb) {
        nimbus_host_free(g_PhysArena); nimbus_host_free(g_PfnDb);
        return STATUS_NO_MEMORY;
    }
    for (i = 0; i < g_PageCount; i++) {
        g_PfnDb[i].Flags = 0x1; /* free */
        g_PfnDb[i].RefCount = 0;
    }
    g_FreeCount = g_PageCount;
    MmBudgetInit();
    MmZramInit();
    MmDedupeInit();
    MmWorkingSetInit();
    MmPagefileInit(PhysicalBytes / 4);
    MmNumaInit();
    g_MmInit = 1;
    return STATUS_SUCCESS;
}

void MmShutdown(void) {
    nimbus_host_free(g_PhysArena); g_PhysArena = NULL;
    nimbus_host_free(g_PfnDb); g_PfnDb = NULL;
    g_PageCount = 0; g_FreeCount = 0; g_MmInit = 0;
}

ULONG MmGetFreePageCount(void) { return g_FreeCount; }

/* Usado por physical_alloc.c */
void MmSetFreeCount(ULONG c) { g_FreeCount = c; }
