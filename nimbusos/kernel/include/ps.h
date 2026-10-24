/* NimbusOS — Process & Thread manager + PE loader. */
#pragma once
#include "ntdef.h"
#include "ke.h"
#include "ob.h"
#include "se.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _EPROCESS {
    KPROCESS        Pcb;
    HANDLE_TABLE    HandleTable;
    PVOID           SectionBase;   /* imagen PE mapeada */
    SIZE_T          ImageSize;
    struct _TOKEN*  Token;
    LONG            HibernateState; /* 0=activo, 1=hibernado */
    ULONG           Affinity;
} EPROCESS, *PEPROCESS;

typedef struct _ETHREAD {
    KTHREAD         Tcb;
    PEPROCESS       Process;
} ETHREAD, *PETHREAD;

NTSTATUS PsInitialize(void);
void     PsShutdown(void);

NTSTATUS PsCreateProcess(PEPROCESS* Out, const char* ImageName, struct _TOKEN* Token);
NTSTATUS PsCreateThread(PEPROCESS Process, PETHREAD* Out, void (*Start)(void*), void* Ctx);
NTSTATUS PsTerminateProcess(PEPROCESS Process);
NTSTATUS PsHibernateProcess(PEPROCESS Process);
NTSTATUS PsWakeProcess(PEPROCESS Process);
void     PsSetAffinity(PETHREAD Thread, ULONG Mask);
void     PsPriorityBoost(PETHREAD Thread, LONG Delta);

/* PE/COFF image load (clean-room parser). */
typedef struct _PE_IMAGE_INFO {
    ULONG     Machine;          /* 0x8664 = AMD64 */
    ULONG     NumberOfSections;
    ULONG     AddressOfEntryPoint;
    ULONG     ImageBase;        /* preferido (32-bit field en PE32+ usamos SizeOfImage) */
    ULONG     SizeOfImage;
    ULONG     SizeOfHeaders;
    ULONG     Subsystem;
    PVOID     MappedBase;       /* base real en host */
    const UCHAR* FileData;
    SIZE_T    FileSize;
} PE_IMAGE_INFO, *PPE_IMAGE_INFO;

NTSTATUS PeParseImage(const void* FileData, SIZE_T FileSize, PPE_IMAGE_INFO Info);
NTSTATUS PeMapImage(PPE_IMAGE_INFO Info, PVOID* OutBase);
NTSTATUS PsLoadPeImage(PEPROCESS Process, const void* FileData, SIZE_T FileSize, PVOID* OutBase);

/* Helpers para fabricar un PE mínimo en tests. */
SIZE_T PeCraftMinimalX64(void* OutBuf, SIZE_T Cap, ULONG EntryRva);

#ifdef __cplusplus
}
#endif
