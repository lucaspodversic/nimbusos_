/* NimbusOS — ALPC (IPC) clean-room. */
#pragma once
#include "ntdef.h"
#ifdef __cplusplus
extern "C" {
#endif
#define ALPC_MAX_MSG 4096
typedef struct _ALPC_PORT {
    CHAR Name[64];
    LONG Connected;
    ULONG MsgHead, MsgTail;
    UCHAR Queue[16][ALPC_MAX_MSG];
    ULONG MsgLen[16];
} ALPC_PORT, *PALPC_PORT;

NTSTATUS AlpcInitialize(void);
NTSTATUS AlpcCreatePort(const char* Name, PALPC_PORT* Out);
NTSTATUS AlpcConnectPort(const char* Name, PALPC_PORT* Out);
NTSTATUS AlpcSend(PALPC_PORT Port, const void* Data, ULONG Len);
NTSTATUS AlpcReceive(PALPC_PORT Port, void* Buf, ULONG Cap, ULONG* OutLen);
NTSTATUS AlpcMapSharedView(PALPC_PORT Port, SIZE_T Size, PVOID* Out);
NTSTATUS AlpcRpcTransportSend(PALPC_PORT Port, const void* Data, ULONG Len);
#ifdef __cplusplus
}
#endif
