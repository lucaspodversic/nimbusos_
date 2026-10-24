/* NimbusOS — power manager. */
#include "io.h"
NTSTATUS IoPowerRequest(PDEVICE_OBJECT Dev, ULONG State) {
    (void)Dev; (void)State; return STATUS_SUCCESS;
}
