/* NimbusOS — auditoría liviana. */
#include "se.h"
#include "../host/host_compat.h"

void SeAuditEvent(const char* Action, const SID* User, NTSTATUS Result) {
    char buf[64];
    SeFormatSid(User, buf, sizeof(buf));
    nimbus_host_printf("AUDIT %s user=%s status=0x%08X\n",
                       Action ? Action : "?", buf, Result);
}
