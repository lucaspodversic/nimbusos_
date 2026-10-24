/* NimbusOS — B-tree de claves del registro (árbol N-ario ordenado por nombre).
 * No es un B-tree clásico de páginas en disco: es un árbol en memoria con
 * orden CM_BTREE_ORDER, suficiente para journal/snapshots y tests.
 */
#include "cm.h"
#include "../host/host_compat.h"

PCM_KEY CmBtreeCreateKey(const char* Name) {
    PCM_KEY k = (PCM_KEY)nimbus_host_calloc(1, sizeof(CM_KEY));
    if (!k) return NULL;
    if (Name) nimbus_host_strncpy(k->Name, Name, CM_KEY_NAME_MAX-1);
    return k;
}

static int name_cmp(const char* a, const char* b) {
    return nimbus_host_strcmp(a, b);
}

/* Inserta hijo manteniendo orden lexicográfico. */
NTSTATUS CmBtreeInsertChild(PCM_KEY Parent, PCM_KEY Child) {
    ULONG i, pos;
    if (!Parent || !Child) return STATUS_INVALID_PARAMETER;
    if (Parent->ChildCount >= CM_BTREE_ORDER) return STATUS_INSUFFICIENT_RESOURCES;
    pos = Parent->ChildCount;
    for (i = 0; i < Parent->ChildCount; i++) {
        if (name_cmp(Child->Name, Parent->Children[i]->Name) < 0) { pos = i; break; }
        if (name_cmp(Child->Name, Parent->Children[i]->Name) == 0)
            return STATUS_OBJECT_NAME_COLLISION;
    }
    for (i = Parent->ChildCount; i > pos; i--)
        Parent->Children[i] = Parent->Children[i-1];
    Parent->Children[pos] = Child;
    Child->Parent = Parent;
    Parent->ChildCount++;
    return STATUS_SUCCESS;
}

PCM_KEY CmBtreeFindChild(PCM_KEY Parent, const char* Name) {
    ULONG i;
    if (!Parent || !Name) return NULL;
    for (i = 0; i < Parent->ChildCount; i++)
        if (name_cmp(Parent->Children[i]->Name, Name) == 0)
            return Parent->Children[i];
    return NULL;
}

void CmBtreeFreeKey(PCM_KEY k) {
    ULONG i;
    if (!k) return;
    for (i = 0; i < k->ChildCount; i++)
        CmBtreeFreeKey(k->Children[i]);
    nimbus_host_free(k);
}

/* Clona un subárbol (para snapshots). */
PCM_KEY CmBtreeClone(PCM_KEY src) {
    PCM_KEY d;
    ULONG i;
    if (!src) return NULL;
    d = CmBtreeCreateKey(src->Name);
    if (!d) return NULL;
    d->ValueCount = src->ValueCount;
    nimbus_host_memcpy(d->Values, src->Values, sizeof(src->Values));
    for (i = 0; i < src->ChildCount; i++) {
        PCM_KEY c = CmBtreeClone(src->Children[i]);
        if (c) CmBtreeInsertChild(d, c);
    }
    return d;
}
