/* NimbusOS — espacio de nombres de objetos (\ObjectTypes\...). */
#include "ob.h"
#include "../host/host_compat.h"

static POBJECT_HEADER g_NamespaceRoot;
static LONG g_ObInit;

NTSTATUS ObInitialize(void) {
    if (g_ObInit) return STATUS_SUCCESS;
    g_NamespaceRoot = NULL;
    g_ObInit = 1;
    return STATUS_SUCCESS;
}

void ObShutdown(void) {
    /* Dangling objects se liberan al bajar refcount en tests. */
    g_NamespaceRoot = NULL;
    g_ObInit = 0;
}

POBJECT_HEADER ObCreateObject(OB_OBJECT_TYPE Type, const char* Name, SIZE_T BodySize) {
    POBJECT_HEADER o = (POBJECT_HEADER)nimbus_host_calloc(1, sizeof(OBJECT_HEADER));
    if (!o) return NULL;
    o->Type = Type;
    o->RefCount = 1;
    o->HandleCount = 0;
    if (Name) nimbus_host_strncpy(o->Name, Name, sizeof(o->Name)-1);
    if (BodySize) {
        o->Body = nimbus_host_calloc(1, BodySize);
        if (!o->Body) { nimbus_host_free(o); return NULL; }
    }
    return o;
}

NTSTATUS ObInsertNamespace(POBJECT_HEADER Obj) {
    if (!Obj) return STATUS_INVALID_PARAMETER;
    Obj->NamespaceNext = g_NamespaceRoot;
    g_NamespaceRoot = Obj;
    ObReferenceObject(Obj);
    return STATUS_SUCCESS;
}

POBJECT_HEADER ObLookupNamespace(const char* Name) {
    POBJECT_HEADER o;
    if (!Name) return NULL;
    for (o = g_NamespaceRoot; o; o = o->NamespaceNext) {
        if (nimbus_host_strcmp(o->Name, Name) == 0)
            return o;
    }
    return NULL;
}
