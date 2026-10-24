/* NimbusOS — journal append-only del hive. */
#include "cm.h"
#include "../host/host_compat.h"

NTSTATUS CmJournalAppend(PCM_HIVE Hive, const CM_JOURNAL_ENTRY* E) {
    if (!Hive || !E) return STATUS_INVALID_PARAMETER;
    if (Hive->JournalCount >= CM_JOURNAL_MAX) return STATUS_INSUFFICIENT_RESOURCES;
    Hive->Journal[Hive->JournalCount++] = *E;
    return STATUS_SUCCESS;
}

NTSTATUS CmJournalFlush(PCM_HIVE Hive) {
    /* En host: el journal ya está en memoria; "flush" es no-op exitoso.
     * En bare-metal se escribiría a NimbusFS. */
    if (!Hive) return STATUS_INVALID_PARAMETER;
    return STATUS_SUCCESS;
}
