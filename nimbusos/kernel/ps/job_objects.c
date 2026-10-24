/* NimbusOS — job objects (límites de proceso agrupados). */
#include "ps.h"
#include "../host/host_compat.h"

typedef struct _JOB {
    ULONG ProcessCount;
    SIZE_T MemoryLimit;
} JOB;

/* Stub funcional mínimo. */
static JOB g_Jobs[16];
