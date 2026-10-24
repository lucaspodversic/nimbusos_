/* NimbusOS — mensajes ALPC. */
#include "alpc.h"
#include "../host/host_compat.h"

NTSTATUS AlpcSend(PALPC_PORT Port, const void* Data, ULONG Len) {
    ULONG next;
    if (!Port || !Data || Len == 0 || Len > ALPC_MAX_MSG) return STATUS_INVALID_PARAMETER;
    next = (Port->MsgTail + 1) % 16;
    if (next == Port->MsgHead) return STATUS_INSUFFICIENT_RESOURCES;
    nimbus_host_memcpy(Port->Queue[Port->MsgTail], Data, Len);
    Port->MsgLen[Port->MsgTail] = Len;
    Port->MsgTail = next;
    return STATUS_SUCCESS;
}
NTSTATUS AlpcReceive(PALPC_PORT Port, void* Buf, ULONG Cap, ULONG* OutLen) {
    ULONG len;
    if (!Port || !Buf || !OutLen) return STATUS_INVALID_PARAMETER;
    if (Port->MsgHead == Port->MsgTail) return STATUS_NOT_FOUND;
    len = Port->MsgLen[Port->MsgHead];
    if (Cap < len) return STATUS_BUFFER_TOO_SMALL;
    nimbus_host_memcpy(Buf, Port->Queue[Port->MsgHead], len);
    *OutLen = len;
    Port->MsgHead = (Port->MsgHead + 1) % 16;
    return STATUS_SUCCESS;
}
