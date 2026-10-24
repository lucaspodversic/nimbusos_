/* NimbusOS — drivers/usb/hub.c (clean-room). */
#include "../include/nimbus_driver.h"
#include <stdlib.h>
#include <string.h>

typedef struct { int Ready; uint64_t Ops; } EXT_hub;

static NTSTATUS disp_hub(PDEVICE_OBJECT Dev, PIRP Irp) {
    EXT_hub* e;
    if (!Dev || !Irp) return STATUS_INVALID_PARAMETER;
    e = (EXT_hub*)Dev->DeviceExtension;
    if (e) e->Ops++;
    Irp->Status = STATUS_SUCCESS;
    Irp->Information = Irp->Length;
    return STATUS_SUCCESS;
}

NTSTATUS hub_DriverEntry(PDRIVER_OBJECT Driver) {
    EXT_hub* e;
    if (!Driver) return STATUS_INVALID_PARAMETER;
    Driver->Name = "hub";
    Driver->MajorFunction[IRP_MJ_CREATE] = disp_hub;
    Driver->MajorFunction[IRP_MJ_READ] = disp_hub;
    Driver->MajorFunction[IRP_MJ_WRITE] = disp_hub;
    Driver->Device = calloc(1, sizeof(DEVICE_OBJECT));
    e = calloc(1, sizeof(*e)); e->Ready = 1;
    Driver->Device->DeviceExtension = e;
    Driver->Device->Name = "\\Device\\hub";
    return STATUS_SUCCESS;
}
