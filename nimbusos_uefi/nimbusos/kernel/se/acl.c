/* NimbusOS — Access Control Lists. */
#include "se.h"
#include "../host/host_compat.h"

void SeInitAcl(PACL Acl) {
    if (Acl) nimbus_host_memset(Acl, 0, sizeof(*Acl));
}

NTSTATUS SeAddAccessAllowedAce(PACL Acl, const SID* Sid, ACCESS_MASK Mask) {
    if (!Acl || !Sid) return STATUS_INVALID_PARAMETER;
    if (Acl->AceCount >= SE_MAX_ACES) return STATUS_INSUFFICIENT_RESOURCES;
    Acl->Aces[Acl->AceCount].Type = AceTypeAllowed;
    Acl->Aces[Acl->AceCount].Mask = Mask;
    SeCopySid(&Acl->Aces[Acl->AceCount].Sid, Sid);
    Acl->AceCount++;
    return STATUS_SUCCESS;
}

NTSTATUS SeAddAccessDeniedAce(PACL Acl, const SID* Sid, ACCESS_MASK Mask) {
    if (!Acl || !Sid) return STATUS_INVALID_PARAMETER;
    if (Acl->AceCount >= SE_MAX_ACES) return STATUS_INSUFFICIENT_RESOURCES;
    Acl->Aces[Acl->AceCount].Type = AceTypeDenied;
    Acl->Aces[Acl->AceCount].Mask = Mask;
    SeCopySid(&Acl->Aces[Acl->AceCount].Sid, Sid);
    Acl->AceCount++;
    return STATUS_SUCCESS;
}
