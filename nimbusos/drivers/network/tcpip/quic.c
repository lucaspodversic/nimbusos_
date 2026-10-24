/* NimbusOS — drivers/network/tcpip/quic.c (clean-room). */
#include "../../include/nimbus_driver.h"
#include <stdlib.h>
#include <string.h>

typedef struct { int Ready; uint64_t Ops; } EXT_quic;

static NTSTATUS disp_quic(PDEVICE_OBJECT Dev, PIRP Irp) {
    EXT_quic* e;
    if (!Dev || !Irp) return STATUS_INVALID_PARAMETER;
    e = (EXT_quic*)Dev->DeviceExtension;
    if (e) e->Ops++;
    Irp->Status = STATUS_SUCCESS;
    Irp->Information = Irp->Length;
    return STATUS_SUCCESS;
}

NTSTATUS quic_DriverEntry(PDRIVER_OBJECT Driver) {
    EXT_quic* e;
    if (!Driver) return STATUS_INVALID_PARAMETER;
    Driver->Name = "quic";
    Driver->MajorFunction[IRP_MJ_CREATE] = disp_quic;
    Driver->MajorFunction[IRP_MJ_READ] = disp_quic;
    Driver->MajorFunction[IRP_MJ_WRITE] = disp_quic;
    Driver->Device = calloc(1, sizeof(DEVICE_OBJECT));
    e = calloc(1, sizeof(*e)); e->Ready = 1;
    Driver->Device->DeviceExtension = e;
    Driver->Device->Name = "\\Device\\quic";
    return STATUS_SUCCESS;
}
