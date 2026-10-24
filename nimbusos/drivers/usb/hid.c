/* NimbusOS — drivers/usb/hid.c (clean-room). */
#include "../include/nimbus_driver.h"
#include <stdlib.h>
#include <string.h>

typedef struct { int Ready; uint64_t Ops; } EXT_hid;

static NTSTATUS disp_hid(PDEVICE_OBJECT Dev, PIRP Irp) {
    EXT_hid* e;
    if (!Dev || !Irp) return STATUS_INVALID_PARAMETER;
    e = (EXT_hid*)Dev->DeviceExtension;
    if (e) e->Ops++;
    Irp->Status = STATUS_SUCCESS;
    Irp->Information = Irp->Length;
    return STATUS_SUCCESS;
}

NTSTATUS hid_DriverEntry(PDRIVER_OBJECT Driver) {
    EXT_hid* e;
    if (!Driver) return STATUS_INVALID_PARAMETER;
    Driver->Name = "hid";
    Driver->MajorFunction[IRP_MJ_CREATE] = disp_hid;
    Driver->MajorFunction[IRP_MJ_READ] = disp_hid;
    Driver->MajorFunction[IRP_MJ_WRITE] = disp_hid;
    Driver->Device = calloc(1, sizeof(DEVICE_OBJECT));
    e = calloc(1, sizeof(*e)); e->Ready = 1;
    Driver->Device->DeviceExtension = e;
    Driver->Device->Name = "\\Device\\hid";
    return STATUS_SUCCESS;
}
