/* NimbusOS — despacho de excepciones (SEH-lite en host). */
#include "ke.h"
#include "../host/host_compat.h"

void KeDispatchException(ULONG Code, void* Address) {
    nimbus_host_printf("NimbusOS exception 0x%08X at %p\n", Code, Address);
    /* En bare-metal se recorrerían frames .pdata; en host solo registramos. */
}
