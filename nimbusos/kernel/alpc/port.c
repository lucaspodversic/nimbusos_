/* NimbusOS — puertos ALPC. */
#include "alpc.h"
#include "../host/host_compat.h"

static ALPC_PORT g_Ports[32];

NTSTATUS AlpcInitialize(void) {
    nimbus_host_memset(g_Ports, 0, sizeof(g_Ports));
    return STATUS_SUCCESS;
}
NTSTATUS AlpcCreatePort(const char* Name, PALPC_PORT* Out) {
    ULONG i;
    for (i = 0; i < 32; i++) {
        if (g_Ports[i].Name[0]) continue;
        nimbus_host_strncpy(g_Ports[i].Name, Name, 63);
        *Out = &g_Ports[i];
        return STATUS_SUCCESS;
    }
    return STATUS_INSUFFICIENT_RESOURCES;
}
NTSTATUS AlpcConnectPort(const char* Name, PALPC_PORT* Out) {
    ULONG i;
    for (i = 0; i < 32; i++) {
        if (nimbus_host_strcmp(g_Ports[i].Name, Name) == 0) {
            g_Ports[i].Connected = 1;
            *Out = &g_Ports[i];
            return STATUS_SUCCESS;
        }
    }
    return STATUS_OBJECT_NAME_NOT_FOUND;
}
