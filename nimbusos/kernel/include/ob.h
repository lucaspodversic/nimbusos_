/* NimbusOS — Object Manager. */
#pragma once
#include "ntdef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _OB_OBJECT_TYPE {
    ObTypeNone = 0,
    ObTypeDirectory,
    ObTypeSymbolicLink,
    ObTypeEvent,
    ObTypeMutex,
    ObTypeSemaphore,
    ObTypeProcess,
    ObTypeThread,
    ObTypeSection,
    ObTypeFile,
    ObTypeKey,
    ObTypePort,
    ObTypeToken,
    ObTypeMax
} OB_OBJECT_TYPE;

typedef struct _OBJECT_HEADER {
    OB_OBJECT_TYPE Type;
    volatile LONG  RefCount;
    volatile LONG  HandleCount;
    CHAR           Name[64];
    void*          Body;
    struct _OBJECT_HEADER* NamespaceNext;
} OBJECT_HEADER, *POBJECT_HEADER;

typedef struct _HANDLE_ENTRY {
    POBJECT_HEADER Object;
    ACCESS_MASK    GrantedAccess;
    LONG           InUse;
} HANDLE_ENTRY, *PHANDLE_ENTRY;

#define OB_HANDLE_TABLE_SIZE 1024

typedef struct _HANDLE_TABLE {
    HANDLE_ENTRY Entries[OB_HANDLE_TABLE_SIZE];
    ULONG        NextFreeHint;
    ULONG        Count;
} HANDLE_TABLE, *PHANDLE_TABLE;

NTSTATUS ObInitialize(void);
void     ObShutdown(void);

POBJECT_HEADER ObCreateObject(OB_OBJECT_TYPE Type, const char* Name, SIZE_T BodySize);
void           ObReferenceObject(POBJECT_HEADER Obj);
void           ObDereferenceObject(POBJECT_HEADER Obj);

NTSTATUS ObInsertHandle(PHANDLE_TABLE Table, POBJECT_HEADER Obj, ACCESS_MASK Access, HANDLE* Out);
NTSTATUS ObReferenceObjectByHandle(PHANDLE_TABLE Table, HANDLE H, ACCESS_MASK Access, POBJECT_HEADER* Out);
NTSTATUS ObCloseHandle(PHANDLE_TABLE Table, HANDLE H);
void     ObInitHandleTable(PHANDLE_TABLE Table);

NTSTATUS ObInsertNamespace(POBJECT_HEADER Obj);
POBJECT_HEADER ObLookupNamespace(const char* Name);
NTSTATUS ObCreateSymbolicLink(const char* LinkName, const char* Target);
POBJECT_HEADER ObResolveSymbolicLink(const char* Name);

const char* ObTypeName(OB_OBJECT_TYPE Type);

#ifdef __cplusplus
}
#endif
