/* xHCI host controller skeleton — colas de transferencia simuladas. */
#include "../include/nimbus_driver.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    int Running;
    uint32_t PortCount;
    uint32_t PortStatus[16]; /* CCS bit0 */
    uint64_t Transfers;
} XHCI_EXT;

static NTSTATUS xhci_dispatch(PDEVICE_OBJECT Dev, PIRP Irp) {
    XHCI_EXT* e = Dev ? (XHCI_EXT*)Dev->DeviceExtension : NULL;
    if (!Irp) return STATUS_INVALID_PARAMETER;
    if (Irp->MajorFunction == IRP_MJ_DEVICE_CONTROL && e) {
        /* IOCTL simulado: enumerar puerto 1 conectado */
        e->PortStatus[0] = 1;
        Irp->Status = STATUS_SUCCESS;
        Irp->Information = sizeof(uint32_t);
        if (Irp->Buffer && Irp->Length >= 4)
            memcpy(Irp->Buffer, &e->PortStatus[0], 4);
        return STATUS_SUCCESS;
    }
    if (Irp->MajorFunction == IRP_MJ_READ && e) {
        e->Transfers++;
        Irp->Status = STATUS_SUCCESS;
        Irp->Information = Irp->Length;
        return STATUS_SUCCESS;
    }
    Irp->Status = STATUS_SUCCESS;
    return STATUS_SUCCESS;
}

NTSTATUS Xhci_DriverEntry(PDRIVER_OBJECT Driver) {
    XHCI_EXT* e;
    if (!Driver) return STATUS_INVALID_PARAMETER;
    Driver->Name = "xhci";
    Driver->MajorFunction[IRP_MJ_CREATE] = xhci_dispatch;
    Driver->MajorFunction[IRP_MJ_READ] = xhci_dispatch;
    Driver->MajorFunction[IRP_MJ_WRITE] = xhci_dispatch;
    Driver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = xhci_dispatch;
    Driver->Device = calloc(1, sizeof(DEVICE_OBJECT));
    e = calloc(1, sizeof(*e));
    e->Running = 1; e->PortCount = 8;
    Driver->Device->Name = "\\Device\\XHCI";
    Driver->Device->DeviceExtension = e;
    return STATUS_SUCCESS;
}
