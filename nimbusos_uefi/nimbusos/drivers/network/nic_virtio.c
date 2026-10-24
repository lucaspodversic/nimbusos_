/* Virtio-net — anillos virtqueue simulados (ideal para QEMU). */
#include "../include/nimbus_driver.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    int Features;
    uint32_t RxPackets, TxPackets;
    uint8_t Mac[6];
} VIRTIO_NET_EXT;

static NTSTATUS virtio_dispatch(PDEVICE_OBJECT Dev, PIRP Irp) {
    VIRTIO_NET_EXT* e = (VIRTIO_NET_EXT*)Dev->DeviceExtension;
    if (Irp->MajorFunction == IRP_MJ_WRITE) {
        e->TxPackets++;
        Irp->Information = Irp->Length;
        Irp->Status = STATUS_SUCCESS;
        return STATUS_SUCCESS;
    }
    if (Irp->MajorFunction == IRP_MJ_READ) {
        e->RxPackets++;
        if (Irp->Buffer && Irp->Length >= 14) {
            /* Ethernet header fake */
            memset(Irp->Buffer, 0, Irp->Length);
            memcpy(Irp->Buffer, e->Mac, 6);
        }
        Irp->Information = Irp->Length;
        Irp->Status = STATUS_SUCCESS;
        return STATUS_SUCCESS;
    }
    Irp->Status = STATUS_SUCCESS;
    return STATUS_SUCCESS;
}

NTSTATUS VirtioNet_DriverEntry(PDRIVER_OBJECT Driver) {
    VIRTIO_NET_EXT* e;
    Driver->Name = "virtio-net";
    Driver->MajorFunction[IRP_MJ_READ] = virtio_dispatch;
    Driver->MajorFunction[IRP_MJ_WRITE] = virtio_dispatch;
    Driver->MajorFunction[IRP_MJ_CREATE] = virtio_dispatch;
    Driver->Device = calloc(1, sizeof(DEVICE_OBJECT));
    e = calloc(1, sizeof(*e));
    e->Features = 1;
    e->Mac[0]=0x52; e->Mac[1]=0x54; e->Mac[2]=0x00;
    e->Mac[3]=0x12; e->Mac[4]=0x34; e->Mac[5]=0x56;
    Driver->Device->DeviceExtension = e;
    Driver->Device->Name = "\\Device\\VirtioNet0";
    return STATUS_SUCCESS;
}
