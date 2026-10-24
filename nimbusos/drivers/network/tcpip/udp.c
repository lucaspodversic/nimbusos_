/* NimbusOS — drivers/network/tcpip/udp.c (clean-room). */
#include "../../include/nimbus_driver.h"
#include <stdlib.h>
#include <string.h>

typedef struct { int Ready; uint64_t Ops; } EXT_udp;

static NTSTATUS disp_udp(PDEVICE_OBJECT Dev, PIRP Irp) {
    EXT_udp* e;
    if (!Dev || !Irp) return STATUS_INVALID_PARAMETER;
    e = (EXT_udp*)Dev->DeviceExtension;
    if (e) e->Ops++;
    Irp->Status = STATUS_SUCCESS;
    Irp->Information = Irp->Length;
    return STATUS_SUCCESS;
}

NTSTATUS udp_DriverEntry(PDRIVER_OBJECT Driver) {
    EXT_udp* e;
    if (!Driver) return STATUS_INVALID_PARAMETER;
    Driver->Name = "udp";
    Driver->MajorFunction[IRP_MJ_CREATE] = disp_udp;
    Driver->MajorFunction[IRP_MJ_READ] = disp_udp;
    Driver->MajorFunction[IRP_MJ_WRITE] = disp_udp;
    Driver->Device = calloc(1, sizeof(DEVICE_OBJECT));
    e = calloc(1, sizeof(*e)); e->Ready = 1;
    Driver->Device->DeviceExtension = e;
    Driver->Device->Name = "\\Device\\udp";
    return STATUS_SUCCESS;
}
