/* Test: advapi32 Reg* round-trip en host. */
#include "win32_host.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    HKEY k = NULL;
    DWORD type = 0, cb;
    char buf[64];
    const char* val = "NimbusOS";
    if (NimbusRegCreateKeyExA(HKEY_LOCAL_MACHINE, "Software\\NimbusOS\\Test", &k) != ERROR_SUCCESS)
        return 1;
    if (NimbusRegSetValueExA(k, "Product", REG_SZ, (const BYTE*)val, (DWORD)strlen(val)+1) != ERROR_SUCCESS)
        return 2;
    cb = sizeof(buf);
    if (NimbusRegQueryValueExA(k, "Product", &type, (BYTE*)buf, &cb) != ERROR_SUCCESS)
        return 3;
    if (type != REG_SZ || strcmp(buf, val) != 0) {
        fprintf(stderr, "got '%s' type=%u\n", buf, type);
        return 4;
    }
    NimbusRegCloseKey(k);
    /* Open existing */
    if (NimbusRegOpenKeyExA(HKEY_LOCAL_MACHINE, "Software\\NimbusOS\\Test", &k) != ERROR_SUCCESS)
        return 5;
    NimbusRegCloseKey(k);
    printf("test_win32_registry OK\n");
    return 0;
}
