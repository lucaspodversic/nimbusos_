/* Almacén content-addressable de DLLs (dedupe). Clean-room NimbusOS. */
#include "../../include/win32_host.h"
#include <string.h>
/* Implementación host mínima: tablas vacías / no-op documentado. */
int nimbus_module_ldr_shared_store_c(void) { return 0; }
