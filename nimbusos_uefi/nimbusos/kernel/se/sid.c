/* NimbusOS — Security Identifiers (SID) clean-room. */
#include "se.h"
#include "../host/host_compat.h"

void SeInitSid(PSID Sid, ULONG Authority, ULONG SubAuth0) {
    if (!Sid) return;
    nimbus_host_memset(Sid, 0, sizeof(*Sid));
    Sid->Revision = 1;
    Sid->SubAuthorityCount = 1;
    /* IdentifierAuthority big-endian en 6 bytes; usamos Authority en el último. */
    Sid->IdentifierAuthority[5] = (UCHAR)(Authority & 0xFF);
    Sid->SubAuthority[0] = SubAuth0;
}

int SeEqualSid(const SID* A, const SID* B) {
    ULONG i;
    if (!A || !B) return 0;
    if (A->Revision != B->Revision) return 0;
    if (A->SubAuthorityCount != B->SubAuthorityCount) return 0;
    if (nimbus_host_memcmp(A->IdentifierAuthority, B->IdentifierAuthority, 6) != 0)
        return 0;
    for (i = 0; i < A->SubAuthorityCount; i++)
        if (A->SubAuthority[i] != B->SubAuthority[i]) return 0;
    return 1;
}

void SeCopySid(PSID Dst, const SID* Src) {
    if (Dst && Src) *Dst = *Src;
}

const char* SeFormatSid(const SID* Sid, char* Buf, SIZE_T Cap) {
    /* Formato S-1-X-Y ... */
    if (!Sid || !Buf || Cap < 16) return NULL;
    /* snprintf puede no estar en freestanding; en host sí. */
#ifdef NIMBUS_HOST
    snprintf(Buf, Cap, "S-%u-%u-%u", Sid->Revision,
             Sid->IdentifierAuthority[5], Sid->SubAuthority[0]);
#else
    (void)Cap;
    Buf[0] = 0;
#endif
    return Buf;
}
