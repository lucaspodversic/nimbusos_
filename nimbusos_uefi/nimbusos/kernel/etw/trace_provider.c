/* NimbusOS — ETW-lite providers. */
#include "etw.h"
#include "../host/host_compat.h"

typedef struct { CHAR Name[32]; LONG InUse; } PROVIDER;
static PROVIDER g_Prov[32];
static ULONG g_NextId = 1;

NTSTATUS EtwInitialize(void) {
    nimbus_host_memset(g_Prov, 0, sizeof(g_Prov));
    g_NextId = 1;
    return STATUS_SUCCESS;
}
NTSTATUS EtwRegisterProvider(const char* Name, ULONG* OutId) {
    ULONG i;
    if (!Name || !OutId) return STATUS_INVALID_PARAMETER;
    for (i = 0; i < 32; i++) {
        if (g_Prov[i].InUse) continue;
        g_Prov[i].InUse = 1;
        nimbus_host_strncpy(g_Prov[i].Name, Name, 31);
        *OutId = g_NextId++;
        return STATUS_SUCCESS;
    }
    return STATUS_INSUFFICIENT_RESOURCES;
}
NTSTATUS EtwWrite(ULONG ProviderId, ULONG EventId, const void* Data, ULONG Len) {
    (void)ProviderId; (void)EventId; (void)Data; (void)Len;
    return STATUS_SUCCESS;
}
