/* NimbusOS — selección de partición A/B. */
#include "bootmgr.h"
static int g_PreferredSlot = 0;
int BootSelectAbSlot(void) { return g_PreferredSlot; }
void BootSetPreferredSlot(int slot) { g_PreferredSlot = slot ? 1 : 0; }
