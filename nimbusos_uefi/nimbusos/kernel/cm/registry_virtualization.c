/* NimbusOS — virtualización de registro por proceso. */
#include "cm.h"
NTSTATUS CmVirtualizeOpen(PCM_HIVE Hive, ULONG ProcessId, const char* Path) {
    (void)Hive; (void)ProcessId; (void)Path;
    return STATUS_SUCCESS;
}
