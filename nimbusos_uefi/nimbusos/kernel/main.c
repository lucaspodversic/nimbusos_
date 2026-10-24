/* NimbusOS — KiSystemStartup: orden de arranque del núcleo.
 *
 * Secuencia (alineada al §6 del diseño):
 *  1. Ke (planificador / sync / syscalls)
 *  2. Mm (memoria, zram, presupuestos)
 *  3. Ob (object manager)
 *  4. Se (seguridad)
 *  5. Ps (procesos)
 *  6. Io (I/O + UMDF)
 *  7. Cm (registro)
 *  8. Cc (cache)
 *  9. Alpc / Etw
 */
#include "nt.h"
#include "alpc.h"
#include "etw.h"
#include "../host/host_compat.h"

NTSTATUS KiSystemStartup(void) {
    NTSTATUS st;
    st = KeInitialize();              if (st) return st;
    st = MmInitialize(64u * 1024u * 1024u); if (st) return st; /* 64 MB arena host */
    st = ObInitialize();              if (st) return st;
    st = SeInitialize();              if (st) return st;
    st = PsInitialize();              if (st) return st;
    st = IoInitialize();              if (st) return st;
    st = IoUmdfHostInit();            if (st) return st;
    st = CmInitialize();              if (st) return st;
    st = CcInitialize();              if (st) return st;
    st = AlpcInitialize();            if (st) return st;
    st = EtwInitialize();             if (st) return st;
    nimbus_host_printf("NimbusOS: KiSystemStartup OK (host=%d)\n",
#ifdef NIMBUS_HOST
                       1
#else
                       0
#endif
    );
    return STATUS_SUCCESS;
}

void KiSystemShutdown(void) {
    CcShutdown();
    CmShutdown();
    IoShutdown();
    PsShutdown();
    SeShutdown();
    ObShutdown();
    MmShutdown();
    KeShutdown();
}
