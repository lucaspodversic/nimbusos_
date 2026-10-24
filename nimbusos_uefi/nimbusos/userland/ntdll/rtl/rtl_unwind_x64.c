/* Unwind x64 — consulta de RUNTIME_FUNCTION (host stub). */
#include "../../include/win32_host.h"
int RtlLookupFunctionEntry(ULONG_PTR pc, ULONG_PTR* imageBase) {
    (void)pc; if (imageBase) *imageBase = 0; return 0;
}
