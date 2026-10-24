/* NimbusOS — hibernación de procesos de UI a zram. */
#include "ps.h"

NTSTATUS PsHibernateProcess(PEPROCESS Process) {
    if (!Process) return STATUS_INVALID_PARAMETER;
    Process->HibernateState = 1;
    return STATUS_SUCCESS;
}
NTSTATUS PsWakeProcess(PEPROCESS Process) {
    if (!Process) return STATUS_INVALID_PARAMETER;
    Process->HibernateState = 0;
    return STATUS_SUCCESS;
}
