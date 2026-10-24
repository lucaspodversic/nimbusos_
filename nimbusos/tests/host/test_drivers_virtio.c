/* Test: virtio-net + nvme + xhci DriverEntry y IRP read/write. */
#include "nimbus_driver.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern NTSTATUS VirtioNet_DriverEntry(PDRIVER_OBJECT Driver);
extern NTSTATUS Nvme_DriverEntry(PDRIVER_OBJECT Driver);
extern NTSTATUS Xhci_DriverEntry(PDRIVER_OBJECT Driver);

static int exercise(PDRIVER_OBJECT drv, const char* name) {
    IRP irp;
    uint8_t buf[64];
    NTSTATUS st;
    memset(&irp, 0, sizeof(irp));
    irp.MajorFunction = IRP_MJ_WRITE;
    irp.Buffer = buf; irp.Length = sizeof(buf);
    st = drv->MajorFunction[IRP_MJ_WRITE](drv->Device, &irp);
    if (st != STATUS_SUCCESS) { fprintf(stderr, "%s write fail\n", name); return 1; }
    irp.MajorFunction = IRP_MJ_READ;
    st = drv->MajorFunction[IRP_MJ_READ](drv->Device, &irp);
    if (st != STATUS_SUCCESS) { fprintf(stderr, "%s read fail\n", name); return 2; }
    printf("  %s OK\n", name);
    return 0;
}

int main(void) {
    DRIVER_OBJECT virtio = {0}, nvme = {0}, xhci = {0};
    if (VirtioNet_DriverEntry(&virtio) != STATUS_SUCCESS) return 1;
    if (Nvme_DriverEntry(&nvme) != STATUS_SUCCESS) return 2;
    if (Xhci_DriverEntry(&xhci) != STATUS_SUCCESS) return 3;
    if (exercise(&virtio, "virtio-net")) return 4;
    if (exercise(&nvme, "nvme")) return 5;
    if (exercise(&xhci, "xhci")) return 6;
    printf("test_drivers_virtio OK\n");
    return 0;
}
