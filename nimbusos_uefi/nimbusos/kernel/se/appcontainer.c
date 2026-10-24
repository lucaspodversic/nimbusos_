/* NimbusOS — AppContainer-style sandbox token. */
#include "se.h"
#include "../host/host_compat.h"

NTSTATUS SeCreateAppContainerToken(PTOKEN* Out, const SID* Capability) {
    SID user;
    NTSTATUS st;
    SeInitSid(&user, 11, 1); /* Authority 11 = AppContainer-ish ficticio */
    st = SeCreateToken(Out, &user, 1 /* low integrity */);
    if (st != STATUS_SUCCESS) return st;
    (*Out)->IsAppContainer = 1;
    if (Capability) SeCopySid(&(*Out)->AppContainerSid, Capability);
    return STATUS_SUCCESS;
}
