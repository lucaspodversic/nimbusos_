/* NimbusOS — tokens de acceso. */
#include "se.h"
#include "../host/host_compat.h"

NTSTATUS SeInitialize(void) { return STATUS_SUCCESS; }
void SeShutdown(void) {}

NTSTATUS SeCreateToken(PTOKEN* Out, const SID* User, ULONG Integrity) {
    PTOKEN t;
    if (!Out || !User) return STATUS_INVALID_PARAMETER;
    t = (PTOKEN)nimbus_host_calloc(1, sizeof(TOKEN));
    if (!t) return STATUS_NO_MEMORY;
    SeCopySid(&t->User, User);
    t->IntegrityLevel = Integrity;
    t->RefCount = 1;
    *Out = t;
    return STATUS_SUCCESS;
}

void SeReferenceToken(PTOKEN Token) {
    if (Token) __sync_fetch_and_add(&Token->RefCount, 1);
}
void SeDereferenceToken(PTOKEN Token) {
    if (Token && __sync_sub_and_fetch(&Token->RefCount, 1) == 0)
        nimbus_host_free(Token);
}

NTSTATUS SeAddGroup(PTOKEN Token, const SID* Group) {
    if (!Token || !Group) return STATUS_INVALID_PARAMETER;
    if (Token->GroupCount >= SE_MAX_GROUPS) return STATUS_INSUFFICIENT_RESOURCES;
    SeCopySid(&Token->Groups[Token->GroupCount++], Group);
    return STATUS_SUCCESS;
}

NTSTATUS SeEnablePrivilege(PTOKEN Token, ULONG PrivBit) {
    if (!Token) return STATUS_INVALID_PARAMETER;
    Token->Privileges |= PrivBit;
    return STATUS_SUCCESS;
}
