/* NimbusOS — drivers/network/tcpip/ip6.c (clean-room). */
#include "../../include/nimbus_driver.h"
#include <stdlib.h>
#include <string.h>

typedef struct { int Ready; uint64_t Ops; } EXT_ip6;

static NTSTATUS disp_ip6(PDEVICE_OBJECT Dev, PIRP Irp) {
    EXT_ip6* e;
    if (!Dev || !Irp) return STATUS_INVALID_PARAMETER;
    e = (EXT_ip6*)Dev->DeviceExtension;
    if (e) e->Ops++;
    Irp->Status = STATUS_SUCCESS;
    Irp->Information = Irp->Length;
    return STATUS_SUCCESS;
}

NTSTATUS ip6_DriverEntry(PDRIVER_OBJECT Driver) {
    EXT_ip6* e;
    if (!Driver) return STATUS_INVALID_PARAMETER;
    Driver->Name = "ip6";
    Driver->MajorFunction[IRP_MJ_CREATE] = disp_ip6;
    Driver->MajorFunction[IRP_MJ_READ] = disp_ip6;
    Driver->MajorFunction[IRP_MJ_WRITE] = disp_ip6;
    Driver->Device = calloc(1, sizeof(DEVICE_OBJECT));
    e = calloc(1, sizeof(*e)); e->Ready = 1;
    Driver->Device->DeviceExtension = e;
    Driver->Device->Name = "\\Device\\ip6";
    return STATUS_SUCCESS;
}
