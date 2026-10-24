/* NimbusOS — SeAccessCheck (modelo NT documentado: DENY antes que ALLOW).
 *
 * Algoritmo:
 *  1. Recorrer ACEs en orden.
 *  2. Si el SID del ACE coincide con User o algún Group del token:
 *     - DENY que solape Desired → ACCESS_DENIED inmediato.
 *     - ALLOW acumula bits en Granted.
 *  3. Éxito si (Granted & Desired) == Desired.
 */
#include "se.h"
#include "../host/host_compat.h"

static int sid_in_token(const TOKEN* Token, const SID* Sid) {
    ULONG i;
    if (SeEqualSid(&Token->User, Sid)) return 1;
    for (i = 0; i < Token->GroupCount; i++)
        if (SeEqualSid(&Token->Groups[i], Sid)) return 1;
    return 0;
}

NTSTATUS SeAccessCheck(const ACL* Acl, const TOKEN* Token,
                       ACCESS_MASK Desired, ACCESS_MASK* Granted) {
    ULONG i;
    ACCESS_MASK got = 0;
    if (!Acl || !Token || !Granted) return STATUS_INVALID_PARAMETER;
    *Granted = 0;
    if (Desired == 0) return STATUS_SUCCESS;

    for (i = 0; i < Acl->AceCount; i++) {
        const ACE* ace = &Acl->Aces[i];
        if (!sid_in_token(Token, &ace->Sid)) continue;
        if (ace->Type == AceTypeDenied) {
            if (ace->Mask & Desired)
                return STATUS_ACCESS_DENIED;
        } else if (ace->Type == AceTypeAllowed) {
            got |= (ace->Mask & Desired);
        }
    }
    *Granted = got;
    if ((got & Desired) == Desired) return STATUS_SUCCESS;
    return STATUS_ACCESS_DENIED;
}
