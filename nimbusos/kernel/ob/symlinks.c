/* NimbusOS — enlaces simbólicos del Object Manager. */
#include "ob.h"
#include "../host/host_compat.h"

typedef struct _OB_SYMLINK {
    CHAR Target[128];
} OB_SYMLINK;

NTSTATUS ObCreateSymbolicLink(const char* LinkName, const char* Target) {
    POBJECT_HEADER o;
    OB_SYMLINK* s;
    if (!LinkName || !Target) return STATUS_INVALID_PARAMETER;
    o = ObCreateObject(ObTypeSymbolicLink, LinkName, sizeof(OB_SYMLINK));
    if (!o) return STATUS_NO_MEMORY;
    s = (OB_SYMLINK*)o->Body;
    nimbus_host_strncpy(s->Target, Target, sizeof(s->Target)-1);
    return ObInsertNamespace(o);
}

POBJECT_HEADER ObResolveSymbolicLink(const char* Name) {
    POBJECT_HEADER o = ObLookupNamespace(Name);
    if (!o) return NULL;
    if (o->Type == ObTypeSymbolicLink) {
        OB_SYMLINK* s = (OB_SYMLINK*)o->Body;
        return ObLookupNamespace(s->Target);
    }
    return o;
}
