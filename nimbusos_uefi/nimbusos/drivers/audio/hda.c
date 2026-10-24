/* NimbusOS — drivers/audio/hda.c (clean-room). */
#include "../include/nimbus_driver.h"
#include <stdlib.h>
#include <string.h>

typedef struct { int Ready; uint64_t Ops; } EXT_hda;

static NTSTATUS disp_hda(PDEVICE_OBJECT Dev, PIRP Irp) {
    EXT_hda* e;
    if (!Dev || !Irp) return STATUS_INVALID_PARAMETER;
    e = (EXT_hda*)Dev->DeviceExtension;
    if (e) e->Ops++;
    Irp->Status = STATUS_SUCCESS;
    Irp->Information = Irp->Length;
    return STATUS_SUCCESS;
}

NTSTATUS hda_DriverEntry(PDRIVER_OBJECT Driver) {
    EXT_hda* e;
    if (!Driver) return STATUS_INVALID_PARAMETER;
    Driver->Name = "hda";
    Driver->MajorFunction[IRP_MJ_CREATE] = disp_hda;
    Driver->MajorFunction[IRP_MJ_READ] = disp_hda;
    Driver->MajorFunction[IRP_MJ_WRITE] = disp_hda;
    Driver->Device = calloc(1, sizeof(DEVICE_OBJECT));
    e = calloc(1, sizeof(*e)); e->Ready = 1;
    Driver->Device->DeviceExtension = e;
    Driver->Device->Name = "\\Device\\hda";
    return STATUS_SUCCESS;
}
