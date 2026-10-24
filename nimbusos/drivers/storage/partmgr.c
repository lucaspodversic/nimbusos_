/* NimbusOS driver: PartMgr — clean-room, compilable en host.
 * En bare-metal habla con el hardware; aquí simula registros/colas.
 */
#include "../include/nimbus_driver.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    int Initialized;
    uint64_t BytesRead;
    uint64_t BytesWritten;
    uint32_t Extra;
} PARTMGR_EXT;

static NTSTATUS PartMgr_dispatch(PDEVICE_OBJECT Dev, PIRP Irp) {
    PARTMGR_EXT* ext;
    if (!Dev || !Irp) return STATUS_INVALID_PARAMETER;
    ext = (PARTMGR_EXT*)Dev->DeviceExtension;
    switch (Irp->MajorFunction) {
    case IRP_MJ_CREATE:
        Irp->Status = STATUS_SUCCESS; break;
    case IRP_MJ_READ:
        if (Irp->Buffer && Irp->Length) {
            memset(Irp->Buffer, 0, Irp->Length);
            if (ext) ext->BytesRead += Irp->Length;
            Irp->Information = Irp->Length;
        }
        Irp->Status = STATUS_SUCCESS; break;
    case IRP_MJ_WRITE:
        if (ext) ext->BytesWritten += Irp->Length;
        Irp->Information = Irp->Length;
        Irp->Status = STATUS_SUCCESS; break;
    default:
        Irp->Status = STATUS_NOT_IMPLEMENTED; break;
    }
    return Irp->Status;
}

NTSTATUS PartMgr_DriverEntry(PDRIVER_OBJECT Driver) {
    PARTMGR_EXT* ext;
    if (!Driver) return STATUS_INVALID_PARAMETER;
    Driver->Name = "PartMgr";
    Driver->MajorFunction[IRP_MJ_CREATE] = PartMgr_dispatch;
    Driver->MajorFunction[IRP_MJ_READ] = PartMgr_dispatch;
    Driver->MajorFunction[IRP_MJ_WRITE] = PartMgr_dispatch;
    Driver->MajorFunction[IRP_MJ_CLOSE] = PartMgr_dispatch;
    Driver->Device = (PDEVICE_OBJECT)calloc(1, sizeof(DEVICE_OBJECT));
    if (!Driver->Device) return STATUS_NO_MEMORY;
    Driver->Device->Name = "\\Device\\PartMgr";
    ext = (PARTMGR_EXT*)calloc(1, sizeof(*ext));
    if (!ext) return STATUS_NO_MEMORY;
    ext->Initialized = 1;
    Driver->Device->DeviceExtension = ext;
    
    return STATUS_SUCCESS;
}
