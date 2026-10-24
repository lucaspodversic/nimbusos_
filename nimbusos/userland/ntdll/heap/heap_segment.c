/* Segment heap: reserva de arenas. */
#include "../../include/win32_host.h"
#include <stdlib.h>
typedef struct { void* base; SIZE_T size; } HEAP_SEGMENT;
HEAP_SEGMENT* RtlCreateHeapSegment(SIZE_T size) {
    HEAP_SEGMENT* s = calloc(1, sizeof(*s));
    if (!s) return NULL;
    s->base = calloc(1, size); s->size = size;
    if (!s->base) { free(s); return NULL; }
    return s;
}
