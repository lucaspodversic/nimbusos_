/* Transporte Virtio-PCI genérico. */
#include "../include/nimbus_driver.h"
#include <stdlib.h>
NTSTATUS VirtioPci_Init(PDRIVER_OBJECT Driver) {
    if (!Driver) return STATUS_INVALID_PARAMETER;
    Driver->Name = "virtio-pci";
    return STATUS_SUCCESS;
}
