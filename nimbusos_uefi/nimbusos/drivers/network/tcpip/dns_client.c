/* NimbusOS — drivers/network/tcpip/dns_client.c (clean-room). */
#include "../../include/nimbus_driver.h"
#include <stdlib.h>
#include <string.h>

typedef struct { int Ready; uint64_t Ops; } EXT_dns_client;

static NTSTATUS disp_dns_client(PDEVICE_OBJECT Dev, PIRP Irp) {
    EXT_dns_client* e;
    if (!Dev || !Irp) return STATUS_INVALID_PARAMETER;
    e = (EXT_dns_client*)Dev->DeviceExtension;
    if (e) e->Ops++;
    Irp->Status = STATUS_SUCCESS;
    Irp->Information = Irp->Length;
    return STATUS_SUCCESS;
}

NTSTATUS dns_client_DriverEntry(PDRIVER_OBJECT Driver) {
    EXT_dns_client* e;
    if (!Driver) return STATUS_INVALID_PARAMETER;
    Driver->Name = "dns_client";
    Driver->MajorFunction[IRP_MJ_CREATE] = disp_dns_client;
    Driver->MajorFunction[IRP_MJ_READ] = disp_dns_client;
    Driver->MajorFunction[IRP_MJ_WRITE] = disp_dns_client;
    Driver->Device = calloc(1, sizeof(DEVICE_OBJECT));
    e = calloc(1, sizeof(*e)); e->Ready = 1;
    Driver->Device->DeviceExtension = e;
    Driver->Device->Name = "\\Device\\dns_client";
    return STATUS_SUCCESS;
}
