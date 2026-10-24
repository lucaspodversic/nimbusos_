/* NimbusOS — niveles de integridad. */
#include "se.h"
ULONG SeGetIntegrityLevel(const TOKEN* Token) {
    return Token ? Token->IntegrityLevel : 0;
}
