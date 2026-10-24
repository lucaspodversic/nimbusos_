/* NimbusOS — tabla de syscalls NT-compatibles (números propios clean-room).
 * Los números NO copian la tabla de Windows; son un ABI propio documentado.
 * ntdll traducirá stubs públicos a estos números.
 */
#include "ke.h"
#include "../host/host_compat.h"

static PSYSCALL_HANDLER g_Table[NIMBUS_SYSCALL_MAX];

void KeSyscallTableInit(void) {
    ULONG i;
    for (i = 0; i < NIMBUS_SYSCALL_MAX; i++) g_Table[i] = NULL;
}

void KeRegisterSyscall(ULONG Number, PSYSCALL_HANDLER Handler) {
    if (Number < NIMBUS_SYSCALL_MAX)
        g_Table[Number] = Handler;
}

NTSTATUS KeInvokeSyscall(ULONG Number, ULONG_PTR a1, ULONG_PTR a2,
                         ULONG_PTR a3, ULONG_PTR a4) {
    if (Number >= NIMBUS_SYSCALL_MAX || !g_Table[Number])
        return STATUS_NOT_IMPLEMENTED;
    return g_Table[Number](a1, a2, a3, a4);
}
