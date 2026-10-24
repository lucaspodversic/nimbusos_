/* Test: enforcement del presupuesto duro por componente. */
#include "test_common.h"
#include "mm.h"

int main(void) {
    NTSTATUS st;
    SIZE_T lim, used;

    EXPECT_STATUS(KiSystemStartup(), STATUS_SUCCESS);

    lim = MmBudgetLimit(NIMBUS_COMP_REGISTRY);
    EXPECT_EQ(lim, 20ull * 1024 * 1024);

    st = MmBudgetTryCharge(NIMBUS_COMP_REGISTRY, 10ull * 1024 * 1024);
    EXPECT_STATUS(st, STATUS_SUCCESS);
    used = MmBudgetUsed(NIMBUS_COMP_REGISTRY);
    EXPECT_EQ(used, 10ull * 1024 * 1024);

    /* Exceder el límite debe fallar. */
    st = MmBudgetTryCharge(NIMBUS_COMP_REGISTRY, 15ull * 1024 * 1024);
    EXPECT_STATUS(st, STATUS_INSUFFICIENT_RESOURCES);
    EXPECT_EQ(MmBudgetUsed(NIMBUS_COMP_REGISTRY), 10ull * 1024 * 1024);

    MmBudgetRelease(NIMBUS_COMP_REGISTRY, 10ull * 1024 * 1024);
    EXPECT_EQ(MmBudgetUsed(NIMBUS_COMP_REGISTRY), 0);

    /* Cargo justo al límite OK */
    st = MmBudgetTryCharge(NIMBUS_COMP_REGISTRY, lim);
    EXPECT_STATUS(st, STATUS_SUCCESS);
    st = MmBudgetTryCharge(NIMBUS_COMP_REGISTRY, 1);
    EXPECT_STATUS(st, STATUS_INSUFFICIENT_RESOURCES);

    KiSystemShutdown();
    if (g_failures) { fprintf(stderr, "%d failures\n", g_failures); return 1; }
    printf("test_memory_budget OK\n");
    return 0;
}
