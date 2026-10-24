/* NimbusOS — drivers/gpu/framebuffer_fallback.c (clean-room). */
#include "../include/nimbus_driver.h"
#include <stdlib.h>
#include <string.h>

typedef struct { int Ready; uint64_t Ops; } EXT_framebuffer_fallback;

static NTSTATUS disp_framebuffer_fallback(PDEVICE_OBJECT Dev, PIRP Irp) {
    EXT_framebuffer_fallback* e;
    if (!Dev || !Irp) return STATUS_INVALID_PARAMETER;
    e = (EXT_framebuffer_fallback*)Dev->DeviceExtension;
    if (e) e->Ops++;
    Irp->Status = STATUS_SUCCESS;
    Irp->Information = Irp->Length;
    return STATUS_SUCCESS;
}

NTSTATUS framebuffer_fallback_DriverEntry(PDRIVER_OBJECT Driver) {
    EXT_framebuffer_fallback* e;
    if (!Driver) return STATUS_INVALID_PARAMETER;
    Driver->Name = "framebuffer_fallback";
    Driver->MajorFunction[IRP_MJ_CREATE] = disp_framebuffer_fallback;
    Driver->MajorFunction[IRP_MJ_READ] = disp_framebuffer_fallback;
    Driver->MajorFunction[IRP_MJ_WRITE] = disp_framebuffer_fallback;
    Driver->Device = calloc(1, sizeof(DEVICE_OBJECT));
    e = calloc(1, sizeof(*e)); e->Ready = 1;
    Driver->Device->DeviceExtension = e;
    Driver->Device->Name = "\\Device\\framebuffer_fallback";
    return STATUS_SUCCESS;
}
