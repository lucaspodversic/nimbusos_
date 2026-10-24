/* NimbusOS — pagefile en disco (simulado en host con buffer). */
#include "mm.h"
#include "../host/host_compat.h"

static UCHAR* g_Pf;
static SIZE_T g_PfSize;
static ULONG  g_PfPages;

NTSTATUS MmPagefileInit(SIZE_T MaxBytes) {
    g_PfSize = MaxBytes;
    g_PfPages = (ULONG)(MaxBytes / PAGE_SIZE);
    g_Pf = (UCHAR*)nimbus_host_calloc(1, MaxBytes ? MaxBytes : PAGE_SIZE);
    return g_Pf ? STATUS_SUCCESS : STATUS_NO_MEMORY;
}

NTSTATUS MmPageOut(ULONG Pfn) {
    extern UCHAR* MmGetPhysArena(void);
    UCHAR* arena = MmGetPhysArena();
    if (!arena || !g_Pf || Pfn >= g_PfPages) return STATUS_INVALID_PARAMETER;
    nimbus_host_memcpy(g_Pf + (SIZE_T)Pfn * PAGE_SIZE, arena + (SIZE_T)Pfn * PAGE_SIZE, PAGE_SIZE);
    return STATUS_SUCCESS;
}

NTSTATUS MmPageIn(ULONG Pfn, void* Out) {
    if (!g_Pf || !Out || Pfn >= g_PfPages) return STATUS_INVALID_PARAMETER;
    nimbus_host_memcpy(Out, g_Pf + (SIZE_T)Pfn * PAGE_SIZE, PAGE_SIZE);
    return STATUS_SUCCESS;
}
