/* NimbusOS — páginas grandes 2 MiB (simulado: N páginas consecutivas). */
#include "mm.h"
#define PAGES_2MB (512u)

NTSTATUS MmAllocateLargePages(ULONG PageCount2MB, ULONG* OutPfn) {
    return MmAllocatePhysicalPages(PageCount2MB * PAGES_2MB, OutPfn);
}
