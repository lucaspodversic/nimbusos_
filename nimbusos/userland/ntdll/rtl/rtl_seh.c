/* RTL SEH — registro de manejadores (host: no-op documentado).
 * En bare-metal x64 usa .pdata / UNWIND_INFO.
 */
#include "../../include/win32_host.h"
typedef int (*PEXCEPTION_HANDLER)(void*);
static PEXCEPTION_HANDLER g_Handler;
void RtlSetUnhandledExceptionFilter(PEXCEPTION_HANDLER h) { g_Handler = h; }
int RtlDispatchException(void* rec) { return g_Handler ? g_Handler(rec) : 0; }
