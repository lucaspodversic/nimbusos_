/* Modelo de driver NimbusOS (UMDF-style / kernel mínimo). Clean-room. */
#ifndef NIMBUS_DRIVER_H
#define NIMBUS_DRIVER_H
#include <stdint.h>
#include <stddef.h>

typedef int32_t NTSTATUS;
#define STATUS_SUCCESS ((NTSTATUS)0)
#define STATUS_NOT_IMPLEMENTED ((NTSTATUS)0xC0000002)
#define STATUS_INVALID_PARAMETER ((NTSTATUS)0xC000000D)
#define STATUS_NO_MEMORY ((NTSTATUS)0xC0000017)
#define STATUS_DEVICE_NOT_READY ((NTSTATUS)0xC00000A3)

typedef struct _DEVICE_OBJECT {
    const char* Name;
    void* DeviceExtension;
    uint32_t Flags;
} DEVICE_OBJECT, *PDEVICE_OBJECT;

typedef struct _IRP {
    uint32_t MajorFunction; /* IRP_MJ_* */
    uint64_t ByteOffset;
    uint32_t Length;
    void* Buffer;
    NTSTATUS Status;
    uint32_t Information;
} IRP, *PIRP;

#define IRP_MJ_CREATE 0
#define IRP_MJ_CLOSE  2
#define IRP_MJ_READ   3
#define IRP_MJ_WRITE  4
#define IRP_MJ_DEVICE_CONTROL 14

typedef NTSTATUS (*PDRIVER_DISPATCH)(PDEVICE_OBJECT, PIRP);

typedef struct _DRIVER_OBJECT {
    const char* Name;
    PDRIVER_DISPATCH MajorFunction[28];
    PDEVICE_OBJECT Device;
} DRIVER_OBJECT, *PDRIVER_OBJECT;

NTSTATUS NimbusDriverRegister(PDRIVER_OBJECT Driver);
#endif
