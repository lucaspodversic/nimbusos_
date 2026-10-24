/* NimbusOS — núcleo del Configuration Manager (registro). */
#include "cm.h"
#include "../host/host_compat.h"

extern PCM_KEY CmBtreeCreateKey(const char* Name);
extern NTSTATUS CmBtreeInsertChild(PCM_KEY Parent, PCM_KEY Child);
extern PCM_KEY CmBtreeFindChild(PCM_KEY Parent, const char* Name);
extern void CmBtreeFreeKey(PCM_KEY k);
extern PCM_KEY CmBtreeClone(PCM_KEY src);
extern NTSTATUS CmJournalAppend(PCM_HIVE Hive, const CM_JOURNAL_ENTRY* E);

static CM_HIVE g_SystemHive;
static LONG g_CmInit;

static PCM_KEY walk_create(PCM_KEY root, const char* Path, LONG Create) {
    /* Path estilo \Registry\Machine\Software\Foo */
    CHAR part[CM_KEY_NAME_MAX];
    const char* p = Path;
    PCM_KEY cur = root;
    if (!root || !Path) return NULL;
    if (*p == '\\' || *p == '/') p++;
    while (*p) {
        SIZE_T n = 0;
        PCM_KEY child;
        while (p[n] && p[n] != '\\' && p[n] != '/') n++;
        if (n >= CM_KEY_NAME_MAX) return NULL;
        nimbus_host_memcpy(part, p, n); part[n] = 0;
        child = CmBtreeFindChild(cur, part);
        if (!child) {
            if (!Create) return NULL;
            child = CmBtreeCreateKey(part);
            if (!child) return NULL;
            if (CmBtreeInsertChild(cur, child) != STATUS_SUCCESS) {
                CmBtreeFreeKey(child); return NULL;
            }
        }
        cur = child;
        p += n;
        if (*p == '\\' || *p == '/') p++;
    }
    return cur;
}

NTSTATUS CmInitialize(void) {
    if (g_CmInit) return STATUS_SUCCESS;
    nimbus_host_memset(&g_SystemHive, 0, sizeof(g_SystemHive));
    nimbus_host_strncpy(g_SystemHive.HiveName, "SYSTEM", sizeof(g_SystemHive.HiveName)-1);
    g_SystemHive.Root = CmBtreeCreateKey("REGISTRY");
    g_CmInit = 1;
    return g_SystemHive.Root ? STATUS_SUCCESS : STATUS_NO_MEMORY;
}

void CmShutdown(void) {
    ULONG i;
    CmBtreeFreeKey(g_SystemHive.Root);
    for (i = 0; i < g_SystemHive.SnapshotCount; i++)
        CmBtreeFreeKey(g_SystemHive.Snapshots[i].RootClone);
    nimbus_host_memset(&g_SystemHive, 0, sizeof(g_SystemHive));
    g_CmInit = 0;
}

PCM_HIVE CmGetSystemHive(void) { return &g_SystemHive; }

NTSTATUS CmCreateHive(const char* Name, PCM_HIVE* Out) {
    PCM_HIVE h;
    if (!Out) return STATUS_INVALID_PARAMETER;
    h = (PCM_HIVE)nimbus_host_calloc(1, sizeof(CM_HIVE));
    if (!h) return STATUS_NO_MEMORY;
    if (Name) nimbus_host_strncpy(h->HiveName, Name, sizeof(h->HiveName)-1);
    h->Root = CmBtreeCreateKey("REGISTRY");
    if (!h->Root) { nimbus_host_free(h); return STATUS_NO_MEMORY; }
    *Out = h;
    return STATUS_SUCCESS;
}

NTSTATUS CmCreateKey(PCM_HIVE Hive, const char* Path) {
    CM_JOURNAL_ENTRY e;
    if (!Hive || !Path) return STATUS_INVALID_PARAMETER;
    if (!walk_create(Hive->Root, Path, 1)) return STATUS_NO_MEMORY;
    nimbus_host_memset(&e, 0, sizeof(e));
    e.Op = CmOpCreateKey;
    nimbus_host_strncpy(e.Path, Path, sizeof(e.Path)-1);
    CmJournalAppend(Hive, &e);
    return STATUS_SUCCESS;
}

NTSTATUS CmSetValue(PCM_HIVE Hive, const char* Path, const char* ValueName,
                    CM_VALUE_TYPE Type, const void* Data, ULONG DataSize) {
    PCM_KEY k;
    ULONG i;
    CM_JOURNAL_ENTRY e;
    if (!Hive || !Path || !ValueName) return STATUS_INVALID_PARAMETER;
    if (DataSize > CM_VALUE_DATA_MAX) return STATUS_BUFFER_TOO_SMALL;
    k = walk_create(Hive->Root, Path, 1);
    if (!k) return STATUS_NO_MEMORY;
    for (i = 0; i < k->ValueCount; i++) {
        if (nimbus_host_strcmp(k->Values[i].Name, ValueName) == 0) {
            k->Values[i].Type = Type;
            k->Values[i].DataSize = DataSize;
            if (Data && DataSize) nimbus_host_memcpy(k->Values[i].Data, Data, DataSize);
            goto journal;
        }
    }
    if (k->ValueCount >= 32) return STATUS_INSUFFICIENT_RESOURCES;
    nimbus_host_strncpy(k->Values[k->ValueCount].Name, ValueName, CM_VALUE_NAME_MAX-1);
    k->Values[k->ValueCount].Type = Type;
    k->Values[k->ValueCount].DataSize = DataSize;
    if (Data && DataSize) nimbus_host_memcpy(k->Values[k->ValueCount].Data, Data, DataSize);
    k->ValueCount++;
journal:
    nimbus_host_memset(&e, 0, sizeof(e));
    e.Op = CmOpSetValue;
    nimbus_host_strncpy(e.Path, Path, sizeof(e.Path)-1);
    nimbus_host_strncpy(e.ValueName, ValueName, sizeof(e.ValueName)-1);
    e.Type = Type;
    e.DataSize = DataSize < sizeof(e.Data) ? DataSize : (ULONG)sizeof(e.Data);
    if (Data) nimbus_host_memcpy(e.Data, Data, e.DataSize);
    CmJournalAppend(Hive, &e);
    return STATUS_SUCCESS;
}

NTSTATUS CmQueryValue(PCM_HIVE Hive, const char* Path, const char* ValueName,
                      CM_VALUE_TYPE* OutType, void* OutData, ULONG* InOutSize) {
    PCM_KEY k;
    ULONG i;
    if (!Hive || !Path || !ValueName || !InOutSize) return STATUS_INVALID_PARAMETER;
    k = walk_create(Hive->Root, Path, 0);
    if (!k) return STATUS_OBJECT_NAME_NOT_FOUND;
    for (i = 0; i < k->ValueCount; i++) {
        if (nimbus_host_strcmp(k->Values[i].Name, ValueName) == 0) {
            if (OutType) *OutType = k->Values[i].Type;
            if (OutData && *InOutSize >= k->Values[i].DataSize)
                nimbus_host_memcpy(OutData, k->Values[i].Data, k->Values[i].DataSize);
            else if (OutData) { *InOutSize = k->Values[i].DataSize; return STATUS_BUFFER_TOO_SMALL; }
            *InOutSize = k->Values[i].DataSize;
            return STATUS_SUCCESS;
        }
    }
    return STATUS_NOT_FOUND;
}

NTSTATUS CmDeleteValue(PCM_HIVE Hive, const char* Path, const char* ValueName) {
    PCM_KEY k; ULONG i, j;
    if (!Hive || !Path || !ValueName) return STATUS_INVALID_PARAMETER;
    k = walk_create(Hive->Root, Path, 0);
    if (!k) return STATUS_OBJECT_NAME_NOT_FOUND;
    for (i = 0; i < k->ValueCount; i++) {
        if (nimbus_host_strcmp(k->Values[i].Name, ValueName) == 0) {
            for (j = i + 1; j < k->ValueCount; j++) k->Values[j-1] = k->Values[j];
            k->ValueCount--;
            return STATUS_SUCCESS;
        }
    }
    return STATUS_NOT_FOUND;
}

NTSTATUS CmDeleteKey(PCM_HIVE Hive, const char* Path) {
    (void)Hive; (void)Path;
    return STATUS_NOT_IMPLEMENTED; /* eliminar hojas en fase posterior */
}

/* Serialización simple: formato binario propio NimbusHive v1. */
#define NIMBUS_HIVE_MAGIC 0x48564D4Eu /* 'NMVH' little sens */

typedef struct _HIVE_FILE_HDR {
    ULONG Magic;
    ULONG Version;
    ULONG KeyCount;
    ULONG ValueCount;
} HIVE_FILE_HDR;

/* Flatten tree into records for round-trip. */
typedef struct _FLAT_KEY {
    CHAR Name[CM_KEY_NAME_MAX];
    ULONG ParentIndex; /* 0 = root's parent sentinel */
    ULONG ValueCount;
} FLAT_KEY;

typedef struct _FLAT_VAL {
    ULONG KeyIndex;
    CHAR Name[CM_VALUE_NAME_MAX];
    ULONG Type;
    ULONG DataSize;
    UCHAR Data[CM_VALUE_DATA_MAX];
} FLAT_VAL;

static void count_tree(PCM_KEY k, ULONG* kc, ULONG* vc) {
    ULONG i;
    if (!k) return;
    (*kc)++;
    *vc += k->ValueCount;
    for (i = 0; i < k->ChildCount; i++) count_tree(k->Children[i], kc, vc);
}

static void flatten(PCM_KEY k, ULONG parent, FLAT_KEY* keys, FLAT_VAL* vals,
                    ULONG* ki, ULONG* vi) {
    ULONG my, i, v;
    if (!k) return;
    my = *ki;
    nimbus_host_memset(&keys[my], 0, sizeof(FLAT_KEY));
    nimbus_host_strncpy(keys[my].Name, k->Name, CM_KEY_NAME_MAX-1);
    keys[my].ParentIndex = parent;
    keys[my].ValueCount = k->ValueCount;
    (*ki)++;
    for (v = 0; v < k->ValueCount; v++) {
        FLAT_VAL* fv = &vals[*vi];
        nimbus_host_memset(fv, 0, sizeof(*fv));
        fv->KeyIndex = my;
        nimbus_host_strncpy(fv->Name, k->Values[v].Name, CM_VALUE_NAME_MAX-1);
        fv->Type = (ULONG)k->Values[v].Type;
        fv->DataSize = k->Values[v].DataSize;
        nimbus_host_memcpy(fv->Data, k->Values[v].Data, k->Values[v].DataSize);
        (*vi)++;
    }
    for (i = 0; i < k->ChildCount; i++)
        flatten(k->Children[i], my, keys, vals, ki, vi);
}

NTSTATUS CmSerializeHive(PCM_HIVE Hive, void* Buf, SIZE_T Cap, SIZE_T* OutSize) {
    ULONG kc = 0, vc = 0, ki = 0, vi = 0;
    SIZE_T need;
    UCHAR* out = (UCHAR*)Buf;
    HIVE_FILE_HDR hdr;
    FLAT_KEY* keys;
    FLAT_VAL* vals;
    if (!Hive || !OutSize) return STATUS_INVALID_PARAMETER;
    count_tree(Hive->Root, &kc, &vc);
    need = sizeof(hdr) + (SIZE_T)kc * sizeof(FLAT_KEY) + (SIZE_T)vc * sizeof(FLAT_VAL);
    *OutSize = need;
    if (!Buf || Cap < need) return STATUS_BUFFER_TOO_SMALL;
    keys = (FLAT_KEY*)nimbus_host_calloc(kc ? kc : 1, sizeof(FLAT_KEY));
    vals = (FLAT_VAL*)nimbus_host_calloc(vc ? vc : 1, sizeof(FLAT_VAL));
    if (!keys || !vals) { nimbus_host_free(keys); nimbus_host_free(vals); return STATUS_NO_MEMORY; }
    flatten(Hive->Root, 0xFFFFFFFFu, keys, vals, &ki, &vi);
    hdr.Magic = NIMBUS_HIVE_MAGIC;
    hdr.Version = 1;
    hdr.KeyCount = kc;
    hdr.ValueCount = vc;
    nimbus_host_memcpy(out, &hdr, sizeof(hdr)); out += sizeof(hdr);
    nimbus_host_memcpy(out, keys, kc * sizeof(FLAT_KEY)); out += kc * sizeof(FLAT_KEY);
    nimbus_host_memcpy(out, vals, vc * sizeof(FLAT_VAL));
    nimbus_host_free(keys); nimbus_host_free(vals);
    return STATUS_SUCCESS;
}

NTSTATUS CmDeserializeHive(const void* Buf, SIZE_T Size, PCM_HIVE* Out) {
    const UCHAR* p = (const UCHAR*)Buf;
    HIVE_FILE_HDR hdr;
    const FLAT_KEY* keys;
    const FLAT_VAL* vals;
    PCM_KEY* map;
    ULONG i, v;
    PCM_HIVE h;
    if (!Buf || !Out || Size < sizeof(hdr)) return STATUS_INVALID_PARAMETER;
    nimbus_host_memcpy(&hdr, p, sizeof(hdr));
    if (hdr.Magic != NIMBUS_HIVE_MAGIC || hdr.Version != 1) return STATUS_INVALID_PARAMETER;
    p += sizeof(hdr);
    if (Size < sizeof(hdr) + hdr.KeyCount*sizeof(FLAT_KEY) + hdr.ValueCount*sizeof(FLAT_VAL))
        return STATUS_BUFFER_TOO_SMALL;
    keys = (const FLAT_KEY*)p; p += hdr.KeyCount * sizeof(FLAT_KEY);
    vals = (const FLAT_VAL*)p;
    h = (PCM_HIVE)nimbus_host_calloc(1, sizeof(CM_HIVE));
    map = (PCM_KEY*)nimbus_host_calloc(hdr.KeyCount ? hdr.KeyCount : 1, sizeof(PCM_KEY));
    if (!h || !map) { nimbus_host_free(h); nimbus_host_free(map); return STATUS_NO_MEMORY; }
    for (i = 0; i < hdr.KeyCount; i++) {
        map[i] = CmBtreeCreateKey(keys[i].Name);
        if (!map[i]) { /* leak parcial OK en test fail */ return STATUS_NO_MEMORY; }
    }
    for (i = 0; i < hdr.KeyCount; i++) {
        if (keys[i].ParentIndex == 0xFFFFFFFFu) {
            h->Root = map[i];
        } else {
            CmBtreeInsertChild(map[keys[i].ParentIndex], map[i]);
        }
    }
    for (v = 0; v < hdr.ValueCount; v++) {
        PCM_KEY k = map[vals[v].KeyIndex];
        ULONG idx = k->ValueCount++;
        nimbus_host_strncpy(k->Values[idx].Name, vals[v].Name, CM_VALUE_NAME_MAX-1);
        k->Values[idx].Type = (CM_VALUE_TYPE)vals[v].Type;
        k->Values[idx].DataSize = vals[v].DataSize;
        nimbus_host_memcpy(k->Values[idx].Data, vals[v].Data, vals[v].DataSize);
    }
    nimbus_host_free(map);
    *Out = h;
    return STATUS_SUCCESS;
}
