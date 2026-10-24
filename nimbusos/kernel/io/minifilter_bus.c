/* NimbusOS — bus único de minifiltros (evita cadenas largas de AV). */
#include "io.h"
#include "../host/host_compat.h"

typedef struct { CHAR Name[32]; PFILTER_CALLBACK Cb; void* Ctx; LONG InUse; } FILTER;
static FILTER g_Filters[16];

NTSTATUS IoMinifilterRegister(const char* Name, PFILTER_CALLBACK Cb, void* Ctx) {
    ULONG i;
    for (i = 0; i < 16; i++) {
        if (g_Filters[i].InUse) continue;
        g_Filters[i].InUse = 1;
        if (Name) nimbus_host_strncpy(g_Filters[i].Name, Name, 31);
        g_Filters[i].Cb = Cb; g_Filters[i].Ctx = Ctx;
        return STATUS_SUCCESS;
    }
    return STATUS_INSUFFICIENT_RESOURCES;
}
NTSTATUS IoMinifilterInvoke(PIRP Irp) {
    ULONG i; NTSTATUS st = STATUS_SUCCESS;
    for (i = 0; i < 16; i++) {
        if (!g_Filters[i].InUse || !g_Filters[i].Cb) continue;
        st = g_Filters[i].Cb(Irp, g_Filters[i].Ctx);
        if (st != STATUS_SUCCESS) return st;
    }
    return st;
}
