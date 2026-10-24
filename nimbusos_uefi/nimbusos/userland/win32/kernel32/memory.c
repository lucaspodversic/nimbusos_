/* VirtualAlloc host. */
#include "../../include/win32_host.h"
#include <stdlib.h>
void* NimbusVirtualAlloc(void* addr, SIZE_T size, DWORD type, DWORD protect) {
    (void)addr; (void)type; (void)protect;
    return calloc(1, size ? size : 1);
}
BOOL NimbusVirtualFree(void* addr, SIZE_T size) { (void)size; free(addr); return TRUE; }
