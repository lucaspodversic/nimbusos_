/* NimbusOS — drivers/network/wifi/fw_loader.c (clean-room). */
#include "../../include/nimbus_driver.h"
#include <stdlib.h>
#include <string.h>

typedef struct { int Ready; uint64_t Ops; } EXT_fw_loader;

static NTSTATUS disp_fw_loader(PDEVICE_OBJECT Dev, PIRP Irp) {
    EXT_fw_loader* e;
    if (!Dev || !Irp) return STATUS_INVALID_PARAMETER;
    e = (EXT_fw_loader*)Dev->DeviceExtension;
    if (e) e->Ops++;
    Irp->Status = STATUS_SUCCESS;
    Irp->Information = Irp->Length;
    return STATUS_SUCCESS;
}

NTSTATUS fw_loader_DriverEntry(PDRIVER_OBJECT Driver) {
    EXT_fw_loader* e;
    if (!Driver) return STATUS_INVALID_PARAMETER;
    Driver->Name = "fw_loader";
    Driver->MajorFunction[IRP_MJ_CREATE] = disp_fw_loader;
    Driver->MajorFunction[IRP_MJ_READ] = disp_fw_loader;
    Driver->MajorFunction[IRP_MJ_WRITE] = disp_fw_loader;
    Driver->Device = calloc(1, sizeof(DEVICE_OBJECT));
    e = calloc(1, sizeof(*e)); e->Ready = 1;
    Driver->Device->DeviceExtension = e;
    Driver->Device->Name = "\\Device\\fw_loader";
    return STATUS_SUCCESS;
}
