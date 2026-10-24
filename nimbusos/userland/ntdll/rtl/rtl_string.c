/* RTL strings. */
#include "../../include/win32_host.h"
#include <string.h>
SIZE_T RtlStringCbLengthA(LPCSTR s, SIZE_T max) {
    SIZE_T n = 0; if (!s) return 0;
    while (n < max && s[n]) n++;
    return n;
}
