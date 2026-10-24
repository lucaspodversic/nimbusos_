/* NimbusOS — presupuestos duros de memoria por componente (tabla §4 del diseño).
 * El techo total del sistema es 1024 MB. Cada cargo se contabiliza; si se
 * excede el límite del componente, MmBudgetTryCharge falla.
 */
#include "mm.h"
#include "../host/host_compat.h"

/* Límites en bytes según el plan (MB → bytes). */
static const SIZE_T g_Limits[NIMBUS_COMP_COUNT] = {
    90ull  * 1024 * 1024,  /* kernel */
    60ull  * 1024 * 1024,  /* cache */
    20ull  * 1024 * 1024,  /* registry */
    60ull  * 1024 * 1024,  /* services */
    120ull * 1024 * 1024,  /* window server */
    70ull  * 1024 * 1024,  /* shell */
    35ull  * 1024 * 1024,  /* audio+net */
    40ull  * 1024 * 1024,  /* security */
    80ull  * 1024 * 1024,  /* gpu buffers */
};

static SIZE_T g_Used[NIMBUS_COMP_COUNT];

NTSTATUS MmBudgetInit(void) {
    nimbus_host_memset(g_Used, 0, sizeof(g_Used));
    return STATUS_SUCCESS;
}

SIZE_T MmBudgetLimit(NIMBUS_COMPONENT_ID Comp) {
    if (Comp < 0 || Comp >= NIMBUS_COMP_COUNT) return 0;
    return g_Limits[Comp];
}

SIZE_T MmBudgetUsed(NIMBUS_COMPONENT_ID Comp) {
    if (Comp < 0 || Comp >= NIMBUS_COMP_COUNT) return 0;
    return g_Used[Comp];
}

NTSTATUS MmBudgetTryCharge(NIMBUS_COMPONENT_ID Comp, SIZE_T Bytes) {
    if (Comp < 0 || Comp >= NIMBUS_COMP_COUNT) return STATUS_INVALID_PARAMETER;
    if (g_Used[Comp] + Bytes > g_Limits[Comp])
        return STATUS_INSUFFICIENT_RESOURCES;
    g_Used[Comp] += Bytes;
    return STATUS_SUCCESS;
}

NTSTATUS MmBudgetCharge(NIMBUS_COMPONENT_ID Comp, SIZE_T Bytes) {
    return MmBudgetTryCharge(Comp, Bytes);
}

void MmBudgetRelease(NIMBUS_COMPONENT_ID Comp, SIZE_T Bytes) {
    if (Comp < 0 || Comp >= NIMBUS_COMP_COUNT) return;
    if (Bytes > g_Used[Comp]) g_Used[Comp] = 0;
    else g_Used[Comp] -= Bytes;
}
