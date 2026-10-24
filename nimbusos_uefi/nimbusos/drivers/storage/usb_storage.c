/* NimbusOS driver: UsbStor — clean-room, compilable en host.
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
} USBSTOR_EXT;

static NTSTATUS UsbStor_dispatch(PDEVICE_OBJECT Dev, PIRP Irp) {
    USBSTOR_EXT* ext;
    if (!Dev || !Irp) return STATUS_INVALID_PARAMETER;
    ext = (USBSTOR_EXT*)Dev->DeviceExtension;
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

NTSTATUS UsbStor_DriverEntry(PDRIVER_OBJECT Driver) {
    USBSTOR_EXT* ext;
    if (!Driver) return STATUS_INVALID_PARAMETER;
    Driver->Name = "UsbStor";
    Driver->MajorFunction[IRP_MJ_CREATE] = UsbStor_dispatch;
    Driver->MajorFunction[IRP_MJ_READ] = UsbStor_dispatch;
    Driver->MajorFunction[IRP_MJ_WRITE] = UsbStor_dispatch;
    Driver->MajorFunction[IRP_MJ_CLOSE] = UsbStor_dispatch;
    Driver->Device = (PDEVICE_OBJECT)calloc(1, sizeof(DEVICE_OBJECT));
    if (!Driver->Device) return STATUS_NO_MEMORY;
    Driver->Device->Name = "\\Device\\UsbStor";
    ext = (USBSTOR_EXT*)calloc(1, sizeof(*ext));
    if (!ext) return STATUS_NO_MEMORY;
    ext->Initialized = 1;
    Driver->Device->DeviceExtension = ext;
    
    return STATUS_SUCCESS;
}
