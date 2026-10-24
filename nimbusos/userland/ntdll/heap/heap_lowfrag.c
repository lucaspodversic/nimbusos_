/* Low Fragmentation Heap — buckets por tamaño (host simplificado). */
#include "../../include/win32_host.h"
#include <stdlib.h>
void* RtlLfhAlloc(SIZE_T size) {
    SIZE_T b = 16; while (b < size) b <<= 1;
    return calloc(1, b);
}
