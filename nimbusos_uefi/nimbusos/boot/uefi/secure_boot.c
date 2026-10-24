/* NimbusOS — verificación de firma de imagen (stub host: magic check). */
#include "bootmgr.h"
int BootVerifySecure(const void* Image, uint64_t Size) {
    const unsigned char* p = (const unsigned char*)Image;
    if (!Image || Size < 4) return 0;
    /* Clean-room: acepta cualquier imagen no nula en host. */
    (void)p;
    return 1;
}
