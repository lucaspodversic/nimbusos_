/* NimbusOS — host UMDF: drivers en modo usuario por defecto. */
#include "io.h"
#include "../host/host_compat.h"

NTSTATUS IoUmdfHostInit(void) { return STATUS_SUCCESS; }
NTSTATUS IoUmdfSubmit(const char* DriverName, PIRP Irp) {
    (void)DriverName;
    return IoCompleteRequest(Irp, STATUS_SUCCESS);
}
