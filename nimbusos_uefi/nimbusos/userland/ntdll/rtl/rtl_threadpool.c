/* Thread pool RTL-lite (host: ejecuta inline). */
#include "../../include/win32_host.h"
typedef void (*PTP_WORK_CB)(void*);
void TpSimpleTryPost(PTP_WORK_CB cb, void* ctx) { if (cb) cb(ctx); }
