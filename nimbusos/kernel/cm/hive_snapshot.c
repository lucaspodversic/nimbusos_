/* NimbusOS — snapshots del registro (clonación de árbol). */
#include "cm.h"
#include "../host/host_compat.h"

extern PCM_KEY CmBtreeClone(PCM_KEY src);
extern void CmBtreeFreeKey(PCM_KEY k);
extern ULONG64 KeQueryInterruptTime(void);

NTSTATUS CmCreateSnapshot(PCM_HIVE Hive, const char* Name) {
    if (!Hive || !Name) return STATUS_INVALID_PARAMETER;
    if (Hive->SnapshotCount >= CM_SNAPSHOT_MAX) return STATUS_INSUFFICIENT_RESOURCES;
    {
        CM_SNAPSHOT* s = &Hive->Snapshots[Hive->SnapshotCount];
        nimbus_host_memset(s, 0, sizeof(*s));
        nimbus_host_strncpy(s->Name, Name, sizeof(s->Name)-1);
        s->RootClone = CmBtreeClone(Hive->Root);
        if (!s->RootClone) return STATUS_NO_MEMORY;
        s->Tick = KeQueryInterruptTime();
        Hive->SnapshotCount++;
    }
    return STATUS_SUCCESS;
}

NTSTATUS CmRestoreSnapshot(PCM_HIVE Hive, const char* Name) {
    ULONG i;
    if (!Hive || !Name) return STATUS_INVALID_PARAMETER;
    for (i = 0; i < Hive->SnapshotCount; i++) {
        if (nimbus_host_strcmp(Hive->Snapshots[i].Name, Name) == 0) {
            CmBtreeFreeKey(Hive->Root);
            Hive->Root = CmBtreeClone(Hive->Snapshots[i].RootClone);
            return Hive->Root ? STATUS_SUCCESS : STATUS_NO_MEMORY;
        }
    }
    return STATUS_NOT_FOUND;
}
