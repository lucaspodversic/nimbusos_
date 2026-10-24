#include "../../include/win32_host.h"
#include <stdlib.h>
typedef struct { int size; char face[32]; } FONT;
FONT* NimbusCreateFontA(int size, const char* face) {
    FONT* f=(FONT*)calloc(1,sizeof(*f)); if(!f)return NULL; f->size=size;
    if(face){ unsigned i=0; while(face[i]&&i<31){f->face[i]=face[i];i++;} }
    return f;
}
