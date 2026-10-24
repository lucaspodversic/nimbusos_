/* NimbusOS — ETW-lite. */
#pragma once
#include "ntdef.h"
#ifdef __cplusplus
extern "C" {
#endif
NTSTATUS EtwInitialize(void);
NTSTATUS EtwRegisterProvider(const char* Name, ULONG* OutId);
NTSTATUS EtwWrite(ULONG ProviderId, ULONG EventId, const void* Data, ULONG Len);
NTSTATUS EtwStartSession(const char* Name);
NTSTATUS EtwStopSession(const char* Name);
ULONGLONG EtwReadCounter(ULONG CounterId);
void EtwIncrementCounter(ULONG CounterId);
#ifdef __cplusplus
}
#endif
