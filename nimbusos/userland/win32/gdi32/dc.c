#include "../../include/win32_host.h"
#include <stdlib.h>
typedef struct { int w,h; DWORD* px; } DC;
HDC NimbusCreateCompatibleDC(HDC hdc) {
    DC* d = (DC*)calloc(1, sizeof(DC)); (void)hdc;
    if (!d) return NULL; d->w=64; d->h=64; d->px=calloc(64*64,4); return (HDC)d;
}
BOOL NimbusDeleteDC(HDC hdc) { DC* d=(DC*)hdc; if(!d)return FALSE; free(d->px); free(d); return TRUE; }
int NimbusFillRectColor(HDC hdc, int x, int y, int w, int h, DWORD color) {
    DC* d=(DC*)hdc; int r,c; if(!d||!d->px) return 0;
    for(r=y;r<y+h && r<d->h;r++) for(c=x;c<x+w && c<d->w;c++)
        if(r>=0&&c>=0) d->px[r*d->w+c]=color;
    return 1;
}
