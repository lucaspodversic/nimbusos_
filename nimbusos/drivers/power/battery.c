/* NimbusOS — drivers/power/battery.c (clean-room). */
#include "../include/nimbus_driver.h"
#include <stdlib.h>
#include <string.h>

typedef struct { int Ready; uint64_t Ops; } EXT_battery;

static NTSTATUS disp_battery(PDEVICE_OBJECT Dev, PIRP Irp) {
    EXT_battery* e;
    if (!Dev || !Irp) return STATUS_INVALID_PARAMETER;
    e = (EXT_battery*)Dev->DeviceExtension;
    if (e) e->Ops++;
    Irp->Status = STATUS_SUCCESS;
    Irp->Information = Irp->Length;
    return STATUS_SUCCESS;
}

NTSTATUS battery_DriverEntry(PDRIVER_OBJECT Driver) {
    EXT_battery* e;
    if (!Driver) return STATUS_INVALID_PARAMETER;
    Driver->Name = "battery";
    Driver->MajorFunction[IRP_MJ_CREATE] = disp_battery;
    Driver->MajorFunction[IRP_MJ_READ] = disp_battery;
    Driver->MajorFunction[IRP_MJ_WRITE] = disp_battery;
    Driver->Device = calloc(1, sizeof(DEVICE_OBJECT));
    e = calloc(1, sizeof(*e)); e->Ready = 1;
    Driver->Device->DeviceExtension = e;
    Driver->Device->Name = "\\Device\\battery";
    return STATUS_SUCCESS;
}
