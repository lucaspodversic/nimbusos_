#include "../../include/win32_host.h"
BOOL NimbusGetMessageA(void* msg) { (void)msg; return FALSE; /* WM_QUIT */ }
BOOL NimbusPostMessageA(HWND hwnd, UINT msg, ULONG_PTR wp, ULONG_PTR lp) {
    (void)hwnd;(void)msg;(void)wp;(void)lp; return TRUE;
}
