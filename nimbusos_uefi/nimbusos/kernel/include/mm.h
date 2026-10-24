/* NimbusOS — Memory Manager (presupuesto ≤1GB, zram, dedupe, COW). */
#pragma once
#include "ntdef.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MM_MAX_PHYSICAL_PAGES   (256u * 1024u)  /* hasta 1 GiB de arena host */
#define MM_ZRAM_SLOT_MAX        4096u

typedef struct _MMPFN {
    ULONG     RefCount;
    ULONG     Flags;       /* bit0=free, bit1=zeroed, bit2=compressed, bit3=shared */
    ULONG     ShareCount;
    ULONG     ZramIndex;   /* índice en zram si comprimida */
    ULONG64   ContentHash; /* para dedupe */
} MMPFN, *PMMPFN;

typedef struct _MM_VAD {
    ULONG_PTR StartVpn;
    ULONG_PTR EndVpn;
    ULONG     Protect;
    LONG      Cow;         /* copy-on-write */
    struct _MM_VAD* Next;
} MM_VAD, *PMM_VAD;

NTSTATUS MmInitialize(SIZE_T PhysicalBytes);
void     MmShutdown(void);

/* Asignación física de páginas. */
NTSTATUS MmAllocatePhysicalPages(ULONG PageCount, ULONG* OutFirstPfn);
void     MmFreePhysicalPages(ULONG FirstPfn, ULONG PageCount);
ULONG    MmGetFreePageCount(void);

/* Virtual: arena lineal en host; en bare-metal sería page tables. */
PVOID    MmAllocateVirtual(SIZE_T Bytes, ULONG Protect);
void     MmFreeVirtual(PVOID Address, SIZE_T Bytes);
NTSTATUS MmProtectVirtual(PVOID Address, SIZE_T Bytes, ULONG NewProtect);

/* Secciones / COW */
NTSTATUS MmCreateSection(PVOID* SectionHandle, SIZE_T Size, ULONG Protect);
NTSTATUS MmMapView(PVOID SectionHandle, PVOID* BaseAddress, SIZE_T* ViewSize, LONG Cow);
NTSTATUS MmUnmapView(PVOID BaseAddress);
NTSTATUS MmCowFault(PVOID Address); /* rompe COW al escribir */

/* Working set */
void MmWorkingSetInit(void);
void MmWorkingSetTrim(ULONG TargetPages);
void MmMemoryPressureNotify(LONG Level);

/* zram */
NTSTATUS MmZramInit(void);
NTSTATUS MmZramCompressPage(ULONG Pfn, const void* PageData);
NTSTATUS MmZramDecompressPage(ULONG Pfn, void* OutPage);
ULONG    MmZramCompressedCount(void);

/* LZ4 mínimo embebido */
SIZE_T MmLz4Compress(const void* Src, SIZE_T SrcLen, void* Dst, SIZE_T DstCap);
SIZE_T MmLz4Decompress(const void* Src, SIZE_T SrcLen, void* Dst, SIZE_T DstCap);
SIZE_T MmZstdCompressStub(const void* Src, SIZE_T SrcLen, void* Dst, SIZE_T DstCap);
SIZE_T MmZstdDecompressStub(const void* Src, SIZE_T SrcLen, void* Dst, SIZE_T DstCap);

/* Dedupe KSM-style */
NTSTATUS MmDedupeInit(void);
NTSTATUS MmDedupeScan(void);
ULONG    MmDedupeSharedPages(void);
ULONG64  MmHashPage(const void* Page);

/* Presupuesto duro por componente */
NTSTATUS MmBudgetInit(void);
NTSTATUS MmBudgetCharge(NIMBUS_COMPONENT_ID Comp, SIZE_T Bytes);
void     MmBudgetRelease(NIMBUS_COMPONENT_ID Comp, SIZE_T Bytes);
SIZE_T   MmBudgetUsed(NIMBUS_COMPONENT_ID Comp);
SIZE_T   MmBudgetLimit(NIMBUS_COMPONENT_ID Comp);
NTSTATUS MmBudgetTryCharge(NIMBUS_COMPONENT_ID Comp, SIZE_T Bytes); /* falla si excede */

/* Pool / large pages / NUMA / KASAN-lite */
PVOID MmAllocatePool(SIZE_T Bytes, NIMBUS_COMPONENT_ID Comp);
void  MmFreePool(PVOID P, SIZE_T Bytes, NIMBUS_COMPONENT_ID Comp);
NTSTATUS MmAllocateLargePages(ULONG PageCount2MB, ULONG* OutPfn);
void     MmNumaInit(void);
void     MmKasanLitePoison(void* P, SIZE_T N);
void     MmKasanLiteUnpoison(void* P, SIZE_T N);

/* Pagefile */
NTSTATUS MmPagefileInit(SIZE_T MaxBytes);
NTSTATUS MmPageOut(ULONG Pfn);
NTSTATUS MmPageIn(ULONG Pfn, void* Out);

void MmHandlePageFault(PVOID Address, LONG Write);

#ifdef __cplusplus
}
#endif
