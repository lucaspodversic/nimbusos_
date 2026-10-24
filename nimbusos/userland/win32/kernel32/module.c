/* LoadLibrary/GetProcAddress — host: tabla estática mínima. */
#include "../../include/win32_host.h"
#include <string.h>
typedef struct { const char* name; void* fn; } EXPORT;
static int dummy(void) { return 42; }
static EXPORT g_Exports[] = {
    { "DummyExport", (void*)dummy },
    { NULL, NULL }
};
static int g_Mod = 1;
HMODULE NimbusLoadLibraryA(LPCSTR name) {
    (void)name; return (HMODULE)&g_Mod;
}
void* NimbusGetProcAddress(HMODULE m, LPCSTR name) {
    int i; (void)m;
    for (i = 0; g_Exports[i].name; i++)
        if (strcmp(g_Exports[i].name, name) == 0) return g_Exports[i].fn;
    return NULL;
}
