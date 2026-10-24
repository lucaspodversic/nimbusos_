/* Inicialización del loader PE (ntdll). */
#include "../../include/win32_host.h"
static int g_LdrInit;
int LdrInitializeThunk(void) {
    g_LdrInit = 1;
    return 1;
}
int LdrIsInitialized(void) { return g_LdrInit; }
