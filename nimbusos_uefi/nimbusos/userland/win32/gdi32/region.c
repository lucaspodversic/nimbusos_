#include "../../include/win32_host.h"
#include <stdlib.h>
typedef struct { int x,y,w,h; } REGION;
REGION* NimbusCreateRectRgn(int x,int y,int w,int h){
    REGION* r=calloc(1,sizeof(*r)); if(r){*r=(REGION){x,y,w,h};} return r;
}
