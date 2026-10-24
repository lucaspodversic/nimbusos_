#include "../../include/win32_host.h"
#include <stdio.h>
BOOL NimbusWriteConsoleA(const char* s) { fputs(s, stdout); return TRUE; }
