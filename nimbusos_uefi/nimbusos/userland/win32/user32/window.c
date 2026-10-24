/* user32 CreateWindow — host: tabla en memoria. */
#include "../../include/win32_host.h"
#include <stdlib.h>
#include <string.h>
typedef struct { int id; char title[128]; int x,y,w,h; int visible; } WIN;
static WIN g_Wins[64]; static int g_Nw;
HWND NimbusCreateWindowExA(LPCSTR cls, LPCSTR title, int x, int y, int w, int h) {
    WIN* p; (void)cls;
    if (g_Nw >= 64) return NULL;
    p = &g_Wins[g_Nw++];
    memset(p, 0, sizeof(*p));
    p->id = g_Nw; p->x=x; p->y=y; p->w=w; p->h=h; p->visible=1;
    if (title) strncpy(p->title, title, 127);
    return (HWND)p;
}
BOOL NimbusDestroyWindow(HWND hwnd) {
    WIN* p = (WIN*)hwnd; if (!p) return FALSE; p->visible = 0; return TRUE;
}
BOOL NimbusShowWindow(HWND hwnd, int cmd) {
    WIN* p = (WIN*)hwnd; if (!p) return FALSE; p->visible = (cmd != 0); return TRUE;
}
