/* NimbusOS — bump allocator freestanding (fase ISO / QEMU).
 *
 * No hay paginación dinámica aquí: un arena estática de 1 MiB en .bss
 * sirve para demostrar el subconjunto Mm en el arranque serie.
 */
#include "mm_early.h"

#define MM_EARLY_HEAP_SIZE (1024u * 1024u)

static uint8_t  g_heap[MM_EARLY_HEAP_SIZE] __attribute__((aligned(16)));
static size_t   g_offset;

void mm_early_init(void) {
    g_offset = 0;
    /* El heap ya está en BSS (cero); no hace falta limpiar. */
}

void *mm_early_alloc(size_t size) {
    /* Alineación 16 bytes (ABI SysV x86_64). */
    size_t aligned = (size + 15u) & ~15u;
    if (g_offset + aligned > MM_EARLY_HEAP_SIZE)
        return 0;
    void *p = &g_heap[g_offset];
    g_offset += aligned;
    return p;
}

size_t mm_early_used(void)  { return g_offset; }
size_t mm_early_total(void) { return MM_EARLY_HEAP_SIZE; }
