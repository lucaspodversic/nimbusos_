/* ws2_32 mínimo host. */
#include "../../include/win32_host.h"
#include <stdlib.h>
static int g_Wsa;
int NimbusWSAStartup(WORD ver) { (void)ver; g_Wsa=1; return 0; }
int NimbusSocket(int af, int type, int proto) {
    (void)af;(void)type;(void)proto;
    if (!g_Wsa) return -1;
    return 3; /* fd fake */
}
int NimbusClosesocket(int s) { (void)s; return 0; }
