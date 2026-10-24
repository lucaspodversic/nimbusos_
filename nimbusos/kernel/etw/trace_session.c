/* NimbusOS — sesiones de traza. */
#include "etw.h"
#include "../host/host_compat.h"
static CHAR g_Session[64];
NTSTATUS EtwStartSession(const char* Name) {
    if (!Name) return STATUS_INVALID_PARAMETER;
    nimbus_host_strncpy(g_Session, Name, sizeof(g_Session)-1);
    return STATUS_SUCCESS;
}
NTSTATUS EtwStopSession(const char* Name) {
    (void)Name; g_Session[0] = 0; return STATUS_SUCCESS;
}
