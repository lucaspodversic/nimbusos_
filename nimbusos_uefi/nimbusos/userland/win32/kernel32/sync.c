#include "../../include/win32_host.h"
#include <stdlib.h>
typedef struct { int signaled; } EVT;
HANDLE NimbusCreateEventA(BOOL manual, BOOL init) {
    EVT* e = (EVT*)calloc(1, sizeof(*e)); if (e) e->signaled = init; return e;
}
BOOL NimbusSetEvent(HANDLE h) { if (!h) return FALSE; ((EVT*)h)->signaled = 1; return TRUE; }
