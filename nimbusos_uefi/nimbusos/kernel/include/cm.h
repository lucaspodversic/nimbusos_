/* NimbusOS — Configuration Manager (registro B-tree + journal + snapshots). */
#pragma once
#include "ntdef.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CM_KEY_NAME_MAX   64
#define CM_VALUE_NAME_MAX 64
#define CM_VALUE_DATA_MAX 4096
#define CM_BTREE_ORDER    8
#define CM_JOURNAL_MAX    4096
#define CM_SNAPSHOT_MAX   16

typedef enum _CM_VALUE_TYPE {
    CmTypeNone = 0,
    CmTypeSz,
    CmTypeDword,
    CmTypeBinary,
    CmTypeMultiSz
} CM_VALUE_TYPE;

typedef struct _CM_VALUE {
    CHAR          Name[CM_VALUE_NAME_MAX];
    CM_VALUE_TYPE Type;
    ULONG         DataSize;
    UCHAR         Data[CM_VALUE_DATA_MAX];
} CM_VALUE, *PCM_VALUE;

typedef struct _CM_KEY {
    CHAR   Name[CM_KEY_NAME_MAX];
    ULONG  ValueCount;
    CM_VALUE Values[32];
    struct _CM_KEY* Children[CM_BTREE_ORDER];
    ULONG  ChildCount;
    struct _CM_KEY* Parent;
} CM_KEY, *PCM_KEY;

typedef enum _CM_JOURNAL_OP {
    CmOpCreateKey = 1,
    CmOpSetValue,
    CmOpDeleteValue,
    CmOpDeleteKey
} CM_JOURNAL_OP;

typedef struct _CM_JOURNAL_ENTRY {
    CM_JOURNAL_OP Op;
    CHAR Path[128];
    CHAR ValueName[CM_VALUE_NAME_MAX];
    CM_VALUE_TYPE Type;
    ULONG DataSize;
    UCHAR Data[256];
} CM_JOURNAL_ENTRY;

typedef struct _CM_SNAPSHOT {
    CHAR    Name[32];
    PCM_KEY RootClone;
    ULONG64 Tick;
} CM_SNAPSHOT;

typedef struct _CM_HIVE {
    PCM_KEY Root;
    CM_JOURNAL_ENTRY Journal[CM_JOURNAL_MAX];
    ULONG JournalCount;
    CM_SNAPSHOT Snapshots[CM_SNAPSHOT_MAX];
    ULONG SnapshotCount;
    CHAR  HiveName[32];
} CM_HIVE, *PCM_HIVE;

NTSTATUS CmInitialize(void);
void     CmShutdown(void);

NTSTATUS CmCreateHive(const char* Name, PCM_HIVE* Out);
NTSTATUS CmCreateKey(PCM_HIVE Hive, const char* Path);
NTSTATUS CmSetValue(PCM_HIVE Hive, const char* Path, const char* ValueName,
                    CM_VALUE_TYPE Type, const void* Data, ULONG DataSize);
NTSTATUS CmQueryValue(PCM_HIVE Hive, const char* Path, const char* ValueName,
                      CM_VALUE_TYPE* OutType, void* OutData, ULONG* InOutSize);
NTSTATUS CmDeleteValue(PCM_HIVE Hive, const char* Path, const char* ValueName);
NTSTATUS CmDeleteKey(PCM_HIVE Hive, const char* Path);

/* Persistencia en buffer (round-trip tests). */
NTSTATUS CmSerializeHive(PCM_HIVE Hive, void* Buf, SIZE_T Cap, SIZE_T* OutSize);
NTSTATUS CmDeserializeHive(const void* Buf, SIZE_T Size, PCM_HIVE* Out);

NTSTATUS CmJournalFlush(PCM_HIVE Hive);
NTSTATUS CmCreateSnapshot(PCM_HIVE Hive, const char* Name);
NTSTATUS CmRestoreSnapshot(PCM_HIVE Hive, const char* Name);

typedef void (*CM_NOTIFY_CB)(const char* Path, void* Ctx);
NTSTATUS CmRegisterNotify(PCM_HIVE Hive, const char* PathPrefix, CM_NOTIFY_CB Cb, void* Ctx);

/* Virtualización de registro por proceso (stub funcional). */
NTSTATUS CmVirtualizeOpen(PCM_HIVE Hive, ULONG ProcessId, const char* Path);

PCM_HIVE CmGetSystemHive(void);

#ifdef __cplusplus
}
#endif
