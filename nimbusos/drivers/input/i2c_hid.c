/* NimbusOS — drivers/input/i2c_hid.c (clean-room). */
#include "../include/nimbus_driver.h"
#include <stdlib.h>
#include <string.h>

typedef struct { int Ready; uint64_t Ops; } EXT_i2c_hid;

static NTSTATUS disp_i2c_hid(PDEVICE_OBJECT Dev, PIRP Irp) {
    EXT_i2c_hid* e;
    if (!Dev || !Irp) return STATUS_INVALID_PARAMETER;
    e = (EXT_i2c_hid*)Dev->DeviceExtension;
    if (e) e->Ops++;
    Irp->Status = STATUS_SUCCESS;
    Irp->Information = Irp->Length;
    return STATUS_SUCCESS;
}

NTSTATUS i2c_hid_DriverEntry(PDRIVER_OBJECT Driver) {
    EXT_i2c_hid* e;
    if (!Driver) return STATUS_INVALID_PARAMETER;
    Driver->Name = "i2c_hid";
    Driver->MajorFunction[IRP_MJ_CREATE] = disp_i2c_hid;
    Driver->MajorFunction[IRP_MJ_READ] = disp_i2c_hid;
    Driver->MajorFunction[IRP_MJ_WRITE] = disp_i2c_hid;
    Driver->Device = calloc(1, sizeof(DEVICE_OBJECT));
    e = calloc(1, sizeof(*e)); e->Ready = 1;
    Driver->Device->DeviceExtension = e;
    Driver->Device->Name = "\\Device\\i2c_hid";
    return STATUS_SUCCESS;
}
