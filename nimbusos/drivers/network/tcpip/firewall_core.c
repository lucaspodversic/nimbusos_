/* NimbusOS — drivers/network/tcpip/firewall_core.c (clean-room). */
#include "../../include/nimbus_driver.h"
#include <stdlib.h>
#include <string.h>

typedef struct { int Ready; uint64_t Ops; } EXT_firewall_core;

static NTSTATUS disp_firewall_core(PDEVICE_OBJECT Dev, PIRP Irp) {
    EXT_firewall_core* e;
    if (!Dev || !Irp) return STATUS_INVALID_PARAMETER;
    e = (EXT_firewall_core*)Dev->DeviceExtension;
    if (e) e->Ops++;
    Irp->Status = STATUS_SUCCESS;
    Irp->Information = Irp->Length;
    return STATUS_SUCCESS;
}

NTSTATUS firewall_core_DriverEntry(PDRIVER_OBJECT Driver) {
    EXT_firewall_core* e;
    if (!Driver) return STATUS_INVALID_PARAMETER;
    Driver->Name = "firewall_core";
    Driver->MajorFunction[IRP_MJ_CREATE] = disp_firewall_core;
    Driver->MajorFunction[IRP_MJ_READ] = disp_firewall_core;
    Driver->MajorFunction[IRP_MJ_WRITE] = disp_firewall_core;
    Driver->Device = calloc(1, sizeof(DEVICE_OBJECT));
    e = calloc(1, sizeof(*e)); e->Ready = 1;
    Driver->Device->DeviceExtension = e;
    Driver->Device->Name = "\\Device\\firewall_core";
    return STATUS_SUCCESS;
}
