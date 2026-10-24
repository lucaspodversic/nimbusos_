/* NimbusOS — referencia atómica de objetos. */
#include "ob.h"
#include "../host/host_compat.h"

void ObReferenceObject(POBJECT_HEADER Obj) {
    if (Obj) __sync_fetch_and_add(&Obj->RefCount, 1);
}

void ObDereferenceObject(POBJECT_HEADER Obj) {
    if (!Obj) return;
    if (__sync_sub_and_fetch(&Obj->RefCount, 1) == 0) {
        nimbus_host_free(Obj->Body);
        nimbus_host_free(Obj);
    }
}
