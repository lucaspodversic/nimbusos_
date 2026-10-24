/* NimbusOS — drivers/tpm/tpm2.c (clean-room). */
#include "../include/nimbus_driver.h"
#include <stdlib.h>
#include <string.h>

typedef struct { int Ready; uint64_t Ops; } EXT_tpm2;

static NTSTATUS disp_tpm2(PDEVICE_OBJECT Dev, PIRP Irp) {
    EXT_tpm2* e;
    if (!Dev || !Irp) return STATUS_INVALID_PARAMETER;
    e = (EXT_tpm2*)Dev->DeviceExtension;
    if (e) e->Ops++;
    Irp->Status = STATUS_SUCCESS;
    Irp->Information = Irp->Length;
    return STATUS_SUCCESS;
}

NTSTATUS tpm2_DriverEntry(PDRIVER_OBJECT Driver) {
    EXT_tpm2* e;
    if (!Driver) return STATUS_INVALID_PARAMETER;
    Driver->Name = "tpm2";
    Driver->MajorFunction[IRP_MJ_CREATE] = disp_tpm2;
    Driver->MajorFunction[IRP_MJ_READ] = disp_tpm2;
    Driver->MajorFunction[IRP_MJ_WRITE] = disp_tpm2;
    Driver->Device = calloc(1, sizeof(DEVICE_OBJECT));
    e = calloc(1, sizeof(*e)); e->Ready = 1;
    Driver->Device->DeviceExtension = e;
    Driver->Device->Name = "\\Device\\tpm2";
    return STATUS_SUCCESS;
}
