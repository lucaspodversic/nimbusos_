/* NimbusOS — drivers/input/precision_touchpad.c (clean-room). */
#include "../include/nimbus_driver.h"
#include <stdlib.h>
#include <string.h>

typedef struct { int Ready; uint64_t Ops; } EXT_precision_touchpad;

static NTSTATUS disp_precision_touchpad(PDEVICE_OBJECT Dev, PIRP Irp) {
    EXT_precision_touchpad* e;
    if (!Dev || !Irp) return STATUS_INVALID_PARAMETER;
    e = (EXT_precision_touchpad*)Dev->DeviceExtension;
    if (e) e->Ops++;
    Irp->Status = STATUS_SUCCESS;
    Irp->Information = Irp->Length;
    return STATUS_SUCCESS;
}

NTSTATUS precision_touchpad_DriverEntry(PDRIVER_OBJECT Driver) {
    EXT_precision_touchpad* e;
    if (!Driver) return STATUS_INVALID_PARAMETER;
    Driver->Name = "precision_touchpad";
    Driver->MajorFunction[IRP_MJ_CREATE] = disp_precision_touchpad;
    Driver->MajorFunction[IRP_MJ_READ] = disp_precision_touchpad;
    Driver->MajorFunction[IRP_MJ_WRITE] = disp_precision_touchpad;
    Driver->Device = calloc(1, sizeof(DEVICE_OBJECT));
    e = calloc(1, sizeof(*e)); e->Ready = 1;
    Driver->Device->DeviceExtension = e;
    Driver->Device->Name = "\\Device\\precision_touchpad";
    return STATUS_SUCCESS;
}
