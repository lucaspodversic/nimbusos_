/* NimbusOS — transporte RPC sobre ALPC. */
#include "alpc.h"
NTSTATUS AlpcRpcTransportSend(PALPC_PORT Port, const void* Data, ULONG Len) {
    return AlpcSend(Port, Data, Len);
}
