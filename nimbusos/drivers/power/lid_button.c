/* NimbusOS — drivers/power/lid_button.c (clean-room). */
#include "../include/nimbus_driver.h"
#include <stdlib.h>
#include <string.h>

typedef struct { int Ready; uint64_t Ops; } EXT_lid_button;

static NTSTATUS disp_lid_button(PDEVICE_OBJECT Dev, PIRP Irp) {
    EXT_lid_button* e;
    if (!Dev || !Irp) return STATUS_INVALID_PARAMETER;
    e = (EXT_lid_button*)Dev->DeviceExtension;
    if (e) e->Ops++;
    Irp->Status = STATUS_SUCCESS;
    Irp->Information = Irp->Length;
    return STATUS_SUCCESS;
}

NTSTATUS lid_button_DriverEntry(PDRIVER_OBJECT Driver) {
    EXT_lid_button* e;
    if (!Driver) return STATUS_INVALID_PARAMETER;
    Driver->Name = "lid_button";
    Driver->MajorFunction[IRP_MJ_CREATE] = disp_lid_button;
    Driver->MajorFunction[IRP_MJ_READ] = disp_lid_button;
    Driver->MajorFunction[IRP_MJ_WRITE] = disp_lid_button;
    Driver->Device = calloc(1, sizeof(DEVICE_OBJECT));
    e = calloc(1, sizeof(*e)); e->Ready = 1;
    Driver->Device->DeviceExtension = e;
    Driver->Device->Name = "\\Device\\lid_button";
    return STATUS_SUCCESS;
}
