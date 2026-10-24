/* NimbusOS — copy-on-write de vistas de sección. */
#include "mm.h"
#include "../host/host_compat.h"

typedef struct _COW_VIEW {
    PVOID  Base;
    SIZE_T Size;
    PVOID  SectionBase;
    LONG   Cow;
    LONG   InUse;
} COW_VIEW;

#define COW_MAX 128
static COW_VIEW g_Views[COW_MAX];

typedef struct _SECTION {
    PVOID  Base;
    SIZE_T Size;
    ULONG  Protect;
    LONG   Ref;
} SECTION;

#define SEC_MAX 64
static SECTION g_Sections[SEC_MAX];

NTSTATUS MmCreateSection(PVOID* SectionHandle, SIZE_T Size, ULONG Protect) {
    ULONG i;
    if (!SectionHandle || Size == 0) return STATUS_INVALID_PARAMETER;
    Size = (Size + PAGE_SIZE - 1) & ~(SIZE_T)(PAGE_SIZE - 1);
    for (i = 0; i < SEC_MAX; i++) {
        if (g_Sections[i].Ref) continue;
        g_Sections[i].Base = nimbus_host_calloc(1, Size);
        if (!g_Sections[i].Base) return STATUS_NO_MEMORY;
        g_Sections[i].Size = Size;
        g_Sections[i].Protect = Protect;
        g_Sections[i].Ref = 1;
        *SectionHandle = &g_Sections[i];
        return STATUS_SUCCESS;
    }
    return STATUS_INSUFFICIENT_RESOURCES;
}

NTSTATUS MmMapView(PVOID SectionHandle, PVOID* BaseAddress, SIZE_T* ViewSize, LONG Cow) {
    SECTION* s = (SECTION*)SectionHandle;
    ULONG i;
    PVOID view;
    if (!s || !BaseAddress) return STATUS_INVALID_PARAMETER;
    view = nimbus_host_malloc(s->Size);
    if (!view) return STATUS_NO_MEMORY;
    nimbus_host_memcpy(view, s->Base, s->Size);
    for (i = 0; i < COW_MAX; i++) {
        if (g_Views[i].InUse) continue;
        g_Views[i].InUse = 1;
        g_Views[i].Base = view;
        g_Views[i].Size = s->Size;
        g_Views[i].SectionBase = s->Base;
        g_Views[i].Cow = Cow ? 1 : 0;
        break;
    }
    *BaseAddress = view;
    if (ViewSize) *ViewSize = s->Size;
    return STATUS_SUCCESS;
}

NTSTATUS MmUnmapView(PVOID BaseAddress) {
    ULONG i;
    for (i = 0; i < COW_MAX; i++) {
        if (g_Views[i].InUse && g_Views[i].Base == BaseAddress) {
            nimbus_host_free(g_Views[i].Base);
            g_Views[i].InUse = 0;
            return STATUS_SUCCESS;
        }
    }
    nimbus_host_free(BaseAddress);
    return STATUS_SUCCESS;
}

NTSTATUS MmCowFault(PVOID Address) {
    ULONG i;
    for (i = 0; i < COW_MAX; i++) {
        UCHAR* b = (UCHAR*)g_Views[i].Base;
        UCHAR* e;
        if (!g_Views[i].InUse || !g_Views[i].Cow) continue;
        e = b + g_Views[i].Size;
        if ((UCHAR*)Address >= b && (UCHAR*)Address < e) {
            /* Ya tenemos copia privada (malloc); solo limpiamos flag COW. */
            g_Views[i].Cow = 0;
            return STATUS_SUCCESS;
        }
    }
    return STATUS_NOT_FOUND;
}
