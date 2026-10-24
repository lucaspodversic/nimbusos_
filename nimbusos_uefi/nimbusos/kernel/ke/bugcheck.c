/* NimbusOS — bugcheck / panic del núcleo. */
#include "ke.h"
#include "../host/host_compat.h"

static LONG g_BugChecked;

void KeBugCheck(ULONG Code, ULONG_PTR a1, ULONG_PTR a2, ULONG_PTR a3, ULONG_PTR a4) {
    g_BugChecked = 1;
    nimbus_host_printf("NimbusOS BUGCHECK 0x%08X (%p %p %p %p)\n",
                       Code, (void*)a1, (void*)a2, (void*)a3, (void*)a4);
#ifdef NIMBUS_HOST
    /* En host no abortamos el proceso entero para no tumbar ctest;
     * los tests pueden consultar el flag. */
    (void)0;
#else
    for (;;) { /* halt */ }
#endif
}

LONG KeWasBugChecked(void) { return g_BugChecked; }
void KeClearBugCheck(void) { g_BugChecked = 0; }
