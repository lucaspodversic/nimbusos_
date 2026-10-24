/* Heap ntdll — wrapper malloc host (en bare-metal: segment heap propio). */
#include "../../include/win32_host.h"
#include <stdlib.h>
void* RtlAllocateHeap(void* heap, DWORD flags, SIZE_T size) {
    (void)heap; (void)flags;
    return malloc(size ? size : 1);
}
BOOL RtlFreeHeap(void* heap, DWORD flags, void* p) {
    (void)heap; (void)flags; free(p); return TRUE;
}
void* RtlReAllocateHeap(void* heap, DWORD flags, void* p, SIZE_T size) {
    (void)heap; (void)flags; return realloc(p, size ? size : 1);
}
