#include "../../include/win32_host.h"
#include <stdlib.h>
typedef struct { DWORD color; int width; } PEN;
PEN* NimbusCreatePen(DWORD color, int width) {
    PEN* p = (PEN*)calloc(1, sizeof(*p)); if(p){p->color=color;p->width=width;} return p;
}
