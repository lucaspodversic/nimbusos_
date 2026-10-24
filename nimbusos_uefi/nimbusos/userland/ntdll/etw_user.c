/* ETW-lite user: Emite eventos a un ring buffer en proceso. */
#include "../include/win32_host.h"
#include <string.h>
static char g_Ring[4096]; static unsigned g_Pos;
void EtwEventWrite(unsigned id, const char* msg) {
    unsigned n = msg ? (unsigned)strlen(msg) : 0;
    if (n > 200) n = 200;
    if (g_Pos + 5 + n >= sizeof(g_Ring)) g_Pos = 0;
    g_Ring[g_Pos++] = (char)id;
    memcpy(g_Ring + g_Pos, &n, 4); g_Pos += 4;
    if (msg) { memcpy(g_Ring + g_Pos, msg, n); g_Pos += n; }
}
