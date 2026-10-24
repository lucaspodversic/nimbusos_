/* NimbusOS — anillo de I/O estilo io_uring. */
#include "io.h"
#include "../host/host_compat.h"

NTSTATUS IoRingCreate(PIO_RING* Out, ULONG Capacity) {
    PIO_RING r;
    if (!Out || Capacity == 0) return STATUS_INVALID_PARAMETER;
    r = (PIO_RING)nimbus_host_calloc(1, sizeof(IO_RING));
    if (!r) return STATUS_NO_MEMORY;
    r->Slots = (PIRP*)nimbus_host_calloc(Capacity, sizeof(PIRP));
    if (!r->Slots) { nimbus_host_free(r); return STATUS_NO_MEMORY; }
    r->Capacity = Capacity;
    *Out = r;
    return STATUS_SUCCESS;
}
NTSTATUS IoRingSubmit(PIO_RING Ring, PIRP Irp) {
    ULONG next;
    if (!Ring || !Irp) return STATUS_INVALID_PARAMETER;
    next = (Ring->Tail + 1) % Ring->Capacity;
    if (next == Ring->Head) return STATUS_INSUFFICIENT_RESOURCES;
    Ring->Slots[Ring->Tail] = Irp;
    Ring->Tail = next;
    return STATUS_SUCCESS;
}
NTSTATUS IoRingComplete(PIO_RING Ring, PIRP* Out) {
    if (!Ring || !Out) return STATUS_INVALID_PARAMETER;
    if (Ring->Head == Ring->Tail) return STATUS_NOT_FOUND;
    *Out = Ring->Slots[Ring->Head];
    Ring->Head = (Ring->Head + 1) % Ring->Capacity;
    return STATUS_SUCCESS;
}
