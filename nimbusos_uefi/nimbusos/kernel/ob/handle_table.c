/* NimbusOS — tabla de handles por proceso.
 * Los HANDLE son índices 1-based (0 = NULL handle).
 */
#include "ob.h"
#include "../host/host_compat.h"

void ObInitHandleTable(PHANDLE_TABLE Table) {
    if (!Table) return;
    nimbus_host_memset(Table, 0, sizeof(*Table));
    Table->NextFreeHint = 1;
}

NTSTATUS ObInsertHandle(PHANDLE_TABLE Table, POBJECT_HEADER Obj,
                        ACCESS_MASK Access, HANDLE* Out) {
    ULONG i;
    if (!Table || !Obj || !Out) return STATUS_INVALID_PARAMETER;
    for (i = 1; i < OB_HANDLE_TABLE_SIZE; i++) {
        ULONG idx = (Table->NextFreeHint + i - 1) % OB_HANDLE_TABLE_SIZE;
        if (idx == 0) continue;
        if (Table->Entries[idx].InUse) continue;
        Table->Entries[idx].InUse = 1;
        Table->Entries[idx].Object = Obj;
        Table->Entries[idx].GrantedAccess = Access;
        ObReferenceObject(Obj);
        Obj->HandleCount++;
        Table->Count++;
        Table->NextFreeHint = (idx + 1) % OB_HANDLE_TABLE_SIZE;
        if (Table->NextFreeHint == 0) Table->NextFreeHint = 1;
        *Out = (HANDLE)(ULONG_PTR)idx;
        return STATUS_SUCCESS;
    }
    return STATUS_INSUFFICIENT_RESOURCES;
}

NTSTATUS ObReferenceObjectByHandle(PHANDLE_TABLE Table, HANDLE H,
                                   ACCESS_MASK Access, POBJECT_HEADER* Out) {
    ULONG idx = (ULONG)(ULONG_PTR)H;
    PHANDLE_ENTRY e;
    if (!Table || !Out || idx == 0 || idx >= OB_HANDLE_TABLE_SIZE)
        return STATUS_INVALID_PARAMETER;
    e = &Table->Entries[idx];
    if (!e->InUse || !e->Object) return STATUS_INVALID_PARAMETER;
    if ((e->GrantedAccess & Access) != Access && Access != 0)
        return STATUS_ACCESS_DENIED;
    ObReferenceObject(e->Object);
    *Out = e->Object;
    return STATUS_SUCCESS;
}

NTSTATUS ObCloseHandle(PHANDLE_TABLE Table, HANDLE H) {
    ULONG idx = (ULONG)(ULONG_PTR)H;
    PHANDLE_ENTRY e;
    if (!Table || idx == 0 || idx >= OB_HANDLE_TABLE_SIZE)
        return STATUS_INVALID_PARAMETER;
    e = &Table->Entries[idx];
    if (!e->InUse) return STATUS_INVALID_PARAMETER;
    e->Object->HandleCount--;
    ObDereferenceObject(e->Object);
    nimbus_host_memset(e, 0, sizeof(*e));
    Table->Count--;
    return STATUS_SUCCESS;
}
