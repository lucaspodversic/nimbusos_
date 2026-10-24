/* NimbusOS — drivers/network/nic_realtek.c (clean-room). */
#include "../include/nimbus_driver.h"
#include <stdlib.h>
#include <string.h>

typedef struct { int Ready; uint64_t Ops; } EXT_nic_realtek;

static NTSTATUS disp_nic_realtek(PDEVICE_OBJECT Dev, PIRP Irp) {
    EXT_nic_realtek* e;
    if (!Dev || !Irp) return STATUS_INVALID_PARAMETER;
    e = (EXT_nic_realtek*)Dev->DeviceExtension;
    if (e) e->Ops++;
    Irp->Status = STATUS_SUCCESS;
    Irp->Information = Irp->Length;
    return STATUS_SUCCESS;
}

NTSTATUS nic_realtek_DriverEntry(PDRIVER_OBJECT Driver) {
    EXT_nic_realtek* e;
    if (!Driver) return STATUS_INVALID_PARAMETER;
    Driver->Name = "nic_realtek";
    Driver->MajorFunction[IRP_MJ_CREATE] = disp_nic_realtek;
    Driver->MajorFunction[IRP_MJ_READ] = disp_nic_realtek;
    Driver->MajorFunction[IRP_MJ_WRITE] = disp_nic_realtek;
    Driver->Device = calloc(1, sizeof(DEVICE_OBJECT));
    e = calloc(1, sizeof(*e)); e->Ready = 1;
    Driver->Device->DeviceExtension = e;
    Driver->Device->Name = "\\Device\\nic_realtek";
    return STATUS_SUCCESS;
}
