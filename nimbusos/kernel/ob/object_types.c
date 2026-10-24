/* NimbusOS — nombres de tipos de objeto. */
#include "ob.h"

const char* ObTypeName(OB_OBJECT_TYPE Type) {
    static const char* names[] = {
        "None","Directory","SymbolicLink","Event","Mutex","Semaphore",
        "Process","Thread","Section","File","Key","Port","Token"
    };
    if (Type < 0 || Type >= ObTypeMax) return "Unknown";
    return names[Type];
}
