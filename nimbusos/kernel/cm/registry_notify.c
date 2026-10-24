/* NimbusOS — notificaciones de cambio de registro. */
#include "cm.h"
#include "../host/host_compat.h"

typedef struct _CM_NOTIFY {
    CHAR Prefix[128];
    CM_NOTIFY_CB Cb;
    void* Ctx;
    LONG InUse;
} CM_NOTIFY;

static CM_NOTIFY g_Notify[32];

NTSTATUS CmRegisterNotify(PCM_HIVE Hive, const char* PathPrefix, CM_NOTIFY_CB Cb, void* Ctx) {
    ULONG i;
    (void)Hive;
    if (!PathPrefix || !Cb) return STATUS_INVALID_PARAMETER;
    for (i = 0; i < 32; i++) {
        if (g_Notify[i].InUse) continue;
        g_Notify[i].InUse = 1;
        nimbus_host_strncpy(g_Notify[i].Prefix, PathPrefix, sizeof(g_Notify[i].Prefix)-1);
        g_Notify[i].Cb = Cb;
        g_Notify[i].Ctx = Ctx;
        return STATUS_SUCCESS;
    }
    return STATUS_INSUFFICIENT_RESOURCES;
}
