/* NimbusOS — spinlock (clean-room).
 * En host usamos CAS con busy-wait corto; en bare-metal sería con pause/cli.
 * Comentado al detalle para auditoría y aprendizaje.
 */
#include "ke.h"
#include "../host/host_compat.h"

/* Inicializa el candado en estado libre (0). */
void KeInitializeSpinLock(PKSPIN_LOCK Lock) {
    if (!Lock) return;
    Lock->Lock = 0;
}

/* Adquiere el candado. En host hacemos spin con hint de relajación. */
void KeAcquireSpinLock(PKSPIN_LOCK Lock) {
    if (!Lock) return;
    for (;;) {
        /* __sync_bool_compare_and_swap: atómico portable GCC/Clang. */
        if (__sync_bool_compare_and_swap(&Lock->Lock, 0, 1))
            return;
#ifdef NIMBUS_HOST
        /* Cede un poco la CPU para no quemar un core en tests. */
        __asm__ __volatile__("pause");
#endif
    }
}

/* Libera el candado con barrera de memoria. */
void KeReleaseSpinLock(PKSPIN_LOCK Lock) {
    if (!Lock) return;
    __sync_lock_release(&Lock->Lock);
}
