/* GDI bitmaps DIB-section lite. */
#include "../../include/win32_host.h"
#include <stdlib.h>
typedef struct { int w,h; DWORD* bits; } BITMAPOBJ;
void* NimbusCreateDIB(int w, int h) {
    BITMAPOBJ* b = calloc(1, sizeof(*b));
    if (!b) return NULL; b->w=w; b->h=h; b->bits=calloc(w*h,4); return b;
}
void NimbusDeleteBitmap(void* p) { BITMAPOBJ* b=p; if(!b)return; free(b->bits); free(b); }
