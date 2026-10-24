/* NimbusOS — drivers/gpu/vulkan_kmd.c (clean-room). */
#include "../include/nimbus_driver.h"
#include <stdlib.h>
#include <string.h>

typedef struct { int Ready; uint64_t Ops; } EXT_vulkan_kmd;

static NTSTATUS disp_vulkan_kmd(PDEVICE_OBJECT Dev, PIRP Irp) {
    EXT_vulkan_kmd* e;
    if (!Dev || !Irp) return STATUS_INVALID_PARAMETER;
    e = (EXT_vulkan_kmd*)Dev->DeviceExtension;
    if (e) e->Ops++;
    Irp->Status = STATUS_SUCCESS;
    Irp->Information = Irp->Length;
    return STATUS_SUCCESS;
}

NTSTATUS vulkan_kmd_DriverEntry(PDRIVER_OBJECT Driver) {
    EXT_vulkan_kmd* e;
    if (!Driver) return STATUS_INVALID_PARAMETER;
    Driver->Name = "vulkan_kmd";
    Driver->MajorFunction[IRP_MJ_CREATE] = disp_vulkan_kmd;
    Driver->MajorFunction[IRP_MJ_READ] = disp_vulkan_kmd;
    Driver->MajorFunction[IRP_MJ_WRITE] = disp_vulkan_kmd;
    Driver->Device = calloc(1, sizeof(DEVICE_OBJECT));
    e = calloc(1, sizeof(*e)); e->Ready = 1;
    Driver->Device->DeviceExtension = e;
    Driver->Device->Name = "\\Device\\vulkan_kmd";
    return STATUS_SUCCESS;
}
