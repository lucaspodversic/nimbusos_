/* NimbusOS — I/O Manager (modelo IRP). */
#pragma once
#include "ntdef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _IRP_MJ {
    IRP_MJ_CREATE = 0,
    IRP_MJ_CLOSE,
    IRP_MJ_READ,
    IRP_MJ_WRITE,
    IRP_MJ_DEVICE_CONTROL,
    IRP_MJ_PNP,
    IRP_MJ_POWER,
    IRP_MJ_MAXIMUM
} IRP_MJ;

typedef struct _IRP {
    IRP_MJ    MajorFunction;
    ULONG     MinorFunction;
    PVOID     DeviceObject;
    PVOID     SystemBuffer;
    ULONG     Length;
    LONGLONG  ByteOffset;
    NTSTATUS  IoStatus;
    ULONG     Information;
    LONG      Pending;
    void    (*Completion)(struct _IRP* Irp, void* Ctx);
    void*     CompletionContext;
    struct _IRP* Next;
} IRP, *PIRP;

typedef struct _DEVICE_OBJECT {
    CHAR  Name[64];
    PVOID DriverObject;
    PVOID DeviceExtension;
    ULONG Flags;
    struct _DEVICE_OBJECT* Next;
} DEVICE_OBJECT, *PDEVICE_OBJECT;

typedef struct _DRIVER_OBJECT {
    CHAR Name[64];
    NTSTATUS (*Dispatch[IRP_MJ_MAXIMUM])(PDEVICE_OBJECT, PIRP);
    NTSTATUS (*AddDevice)(struct _DRIVER_OBJECT*, PDEVICE_OBJECT);
} DRIVER_OBJECT, *PDRIVER_OBJECT;

NTSTATUS IoInitialize(void);
void     IoShutdown(void);

PIRP     IoAllocateIrp(IRP_MJ Major, ULONG Length);
void     IoFreeIrp(PIRP Irp);
NTSTATUS IoCallDriver(PDEVICE_OBJECT Device, PIRP Irp);
NTSTATUS IoCompleteRequest(PIRP Irp, NTSTATUS Status);

NTSTATUS IoCreateDevice(PDRIVER_OBJECT Driver, const char* Name, PDEVICE_OBJECT* Out);
NTSTATUS IoRegisterDriver(PDRIVER_OBJECT Driver);

/* UMDF host — drivers en modo usuario (simulado en host). */
NTSTATUS IoUmdfHostInit(void);
NTSTATUS IoUmdfSubmit(const char* DriverName, PIRP Irp);

/* Minifilter bus único */
typedef NTSTATUS (*PFILTER_CALLBACK)(PIRP Irp, void* Ctx);
NTSTATUS IoMinifilterRegister(const char* Name, PFILTER_CALLBACK Cb, void* Ctx);
NTSTATUS IoMinifilterInvoke(PIRP Irp);

/* PnP / Power / DMA / io_uring-style */
NTSTATUS IoPnPNotify(PDEVICE_OBJECT Dev, ULONG EventCode);
NTSTATUS IoPowerRequest(PDEVICE_OBJECT Dev, ULONG State);
NTSTATUS IoDmaAllocate(SIZE_T Bytes, PVOID* Out);
void     IoDmaFree(PVOID P, SIZE_T Bytes);

typedef struct _IO_RING {
    ULONG Capacity;
    ULONG Head;
    ULONG Tail;
    PIRP* Slots;
} IO_RING, *PIO_RING;

NTSTATUS IoRingCreate(PIO_RING* Out, ULONG Capacity);
NTSTATUS IoRingSubmit(PIO_RING Ring, PIRP Irp);
NTSTATUS IoRingComplete(PIO_RING Ring, PIRP* Out);

void IoBuildDeviceTree(void);

#ifdef __cplusplus
}
#endif
