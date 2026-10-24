/* NimbusOS — Plug and Play. */
#include "io.h"
NTSTATUS IoPnPNotify(PDEVICE_OBJECT Dev, ULONG EventCode) {
    (void)Dev; (void)EventCode; return STATUS_SUCCESS;
}
