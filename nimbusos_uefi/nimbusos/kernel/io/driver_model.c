/* NimbusOS — modelo de drivers. */
#include "io.h"
#include "../host/host_compat.h"

static PDRIVER_OBJECT g_Drivers;
static PDEVICE_OBJECT g_Devices;

NTSTATUS IoInitialize(void) { g_Drivers = NULL; g_Devices = NULL; return STATUS_SUCCESS; }
void IoShutdown(void) {}

NTSTATUS IoRegisterDriver(PDRIVER_OBJECT Driver) {
    if (!Driver) return STATUS_INVALID_PARAMETER;
    Driver->AddDevice = Driver->AddDevice; /* no-op keep */
    return STATUS_SUCCESS;
}

NTSTATUS IoCreateDevice(PDRIVER_OBJECT Driver, const char* Name, PDEVICE_OBJECT* Out) {
    PDEVICE_OBJECT d = (PDEVICE_OBJECT)nimbus_host_calloc(1, sizeof(DEVICE_OBJECT));
    if (!d) return STATUS_NO_MEMORY;
    if (Name) nimbus_host_strncpy(d->Name, Name, sizeof(d->Name)-1);
    d->DriverObject = Driver;
    d->Next = g_Devices;
    g_Devices = d;
    *Out = d;
    return STATUS_SUCCESS;
}

void IoBuildDeviceTree(void) { /* enumeración PnP simplificada */ }
