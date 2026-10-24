/* NimbusOS — drivers/input/touchscreen.c (clean-room). */
#include "../include/nimbus_driver.h"
#include <stdlib.h>
#include <string.h>

typedef struct { int Ready; uint64_t Ops; } EXT_touchscreen;

static NTSTATUS disp_touchscreen(PDEVICE_OBJECT Dev, PIRP Irp) {
    EXT_touchscreen* e;
    if (!Dev || !Irp) return STATUS_INVALID_PARAMETER;
    e = (EXT_touchscreen*)Dev->DeviceExtension;
    if (e) e->Ops++;
    Irp->Status = STATUS_SUCCESS;
    Irp->Information = Irp->Length;
    return STATUS_SUCCESS;
}

NTSTATUS touchscreen_DriverEntry(PDRIVER_OBJECT Driver) {
    EXT_touchscreen* e;
    if (!Driver) return STATUS_INVALID_PARAMETER;
    Driver->Name = "touchscreen";
    Driver->MajorFunction[IRP_MJ_CREATE] = disp_touchscreen;
    Driver->MajorFunction[IRP_MJ_READ] = disp_touchscreen;
    Driver->MajorFunction[IRP_MJ_WRITE] = disp_touchscreen;
    Driver->Device = calloc(1, sizeof(DEVICE_OBJECT));
    e = calloc(1, sizeof(*e)); e->Ready = 1;
    Driver->Device->DeviceExtension = e;
    Driver->Device->Name = "\\Device\\touchscreen";
    return STATUS_SUCCESS;
}
