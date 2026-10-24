/* NimbusOS — drivers/network/wifi/wifi_core.c (clean-room). */
#include "../../include/nimbus_driver.h"
#include <stdlib.h>
#include <string.h>

typedef struct { int Ready; uint64_t Ops; } EXT_wifi_core;

static NTSTATUS disp_wifi_core(PDEVICE_OBJECT Dev, PIRP Irp) {
    EXT_wifi_core* e;
    if (!Dev || !Irp) return STATUS_INVALID_PARAMETER;
    e = (EXT_wifi_core*)Dev->DeviceExtension;
    if (e) e->Ops++;
    Irp->Status = STATUS_SUCCESS;
    Irp->Information = Irp->Length;
    return STATUS_SUCCESS;
}

NTSTATUS wifi_core_DriverEntry(PDRIVER_OBJECT Driver) {
    EXT_wifi_core* e;
    if (!Driver) return STATUS_INVALID_PARAMETER;
    Driver->Name = "wifi_core";
    Driver->MajorFunction[IRP_MJ_CREATE] = disp_wifi_core;
    Driver->MajorFunction[IRP_MJ_READ] = disp_wifi_core;
    Driver->MajorFunction[IRP_MJ_WRITE] = disp_wifi_core;
    Driver->Device = calloc(1, sizeof(DEVICE_OBJECT));
    e = calloc(1, sizeof(*e)); e->Ready = 1;
    Driver->Device->DeviceExtension = e;
    Driver->Device->Name = "\\Device\\wifi_core";
    return STATUS_SUCCESS;
}
