/* NimbusOS — drivers/audio/bluetooth_a2dp.c (clean-room). */
#include "../include/nimbus_driver.h"
#include <stdlib.h>
#include <string.h>

typedef struct { int Ready; uint64_t Ops; } EXT_bluetooth_a2dp;

static NTSTATUS disp_bluetooth_a2dp(PDEVICE_OBJECT Dev, PIRP Irp) {
    EXT_bluetooth_a2dp* e;
    if (!Dev || !Irp) return STATUS_INVALID_PARAMETER;
    e = (EXT_bluetooth_a2dp*)Dev->DeviceExtension;
    if (e) e->Ops++;
    Irp->Status = STATUS_SUCCESS;
    Irp->Information = Irp->Length;
    return STATUS_SUCCESS;
}

NTSTATUS bluetooth_a2dp_DriverEntry(PDRIVER_OBJECT Driver) {
    EXT_bluetooth_a2dp* e;
    if (!Driver) return STATUS_INVALID_PARAMETER;
    Driver->Name = "bluetooth_a2dp";
    Driver->MajorFunction[IRP_MJ_CREATE] = disp_bluetooth_a2dp;
    Driver->MajorFunction[IRP_MJ_READ] = disp_bluetooth_a2dp;
    Driver->MajorFunction[IRP_MJ_WRITE] = disp_bluetooth_a2dp;
    Driver->Device = calloc(1, sizeof(DEVICE_OBJECT));
    e = calloc(1, sizeof(*e)); e->Ready = 1;
    Driver->Device->DeviceExtension = e;
    Driver->Device->Name = "\\Device\\bluetooth_a2dp";
    return STATUS_SUCCESS;
}
