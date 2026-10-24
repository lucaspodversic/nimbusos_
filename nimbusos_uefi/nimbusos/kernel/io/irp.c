/* NimbusOS — I/O Request Packets. */
#include "io.h"
#include "../host/host_compat.h"

PIRP IoAllocateIrp(IRP_MJ Major, ULONG Length) {
    PIRP irp = (PIRP)nimbus_host_calloc(1, sizeof(IRP));
    if (!irp) return NULL;
    irp->MajorFunction = Major;
    irp->Length = Length;
    return irp;
}
void IoFreeIrp(PIRP Irp) { nimbus_host_free(Irp); }

NTSTATUS IoCompleteRequest(PIRP Irp, NTSTATUS Status) {
    if (!Irp) return STATUS_INVALID_PARAMETER;
    Irp->IoStatus = Status;
    Irp->Pending = 0;
    if (Irp->Completion) Irp->Completion(Irp, Irp->CompletionContext);
    return Status;
}

NTSTATUS IoCallDriver(PDEVICE_OBJECT Device, PIRP Irp) {
    PDRIVER_OBJECT drv;
    if (!Device || !Irp) return STATUS_INVALID_PARAMETER;
    Irp->DeviceObject = Device;
    drv = (PDRIVER_OBJECT)Device->DriverObject;
    if (drv && Irp->MajorFunction < IRP_MJ_MAXIMUM && drv->Dispatch[Irp->MajorFunction])
        return drv->Dispatch[Irp->MajorFunction](Device, Irp);
    return IoCompleteRequest(Irp, STATUS_NOT_IMPLEMENTED);
}
