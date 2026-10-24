/* NimbusOS — working set tracking (simplificado). */
#include "mm.h"
#include "../host/host_compat.h"

static ULONG g_WsPages;

void MmWorkingSetInit(void) { g_WsPages = 0; }
void MmWorkingSetAdd(ULONG Pages) { g_WsPages += Pages; }
void MmWorkingSetSet(ULONG v) { g_WsPages = v; }
ULONG MmWorkingSetGet(void) { return g_WsPages; }
