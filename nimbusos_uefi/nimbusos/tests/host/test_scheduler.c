/* Test: cola de listos del planificador por prioridad. */
#include "test_common.h"
#include "ke.h"

KREADY_QUEUE* KeGetReadyQueue(void);

static void dummy_start(void* c) { (void)c; }

int main(void) {
    KTHREAD t_low, t_mid, t_high, *got;

    EXPECT_STATUS(KiSystemStartup(), STATUS_SUCCESS);

    memset(&t_low, 0, sizeof(t_low));
    memset(&t_mid, 0, sizeof(t_mid));
    memset(&t_high, 0, sizeof(t_high));
    t_low.Priority = 2;
    t_mid.Priority = 10;
    t_high.Priority = 20;

    EXPECT_STATUS(KeStartThread(&t_low, dummy_start, NULL), STATUS_SUCCESS);
    EXPECT_STATUS(KeStartThread(&t_mid, dummy_start, NULL), STATUS_SUCCESS);
    EXPECT_STATUS(KeStartThread(&t_high, dummy_start, NULL), STATUS_SUCCESS);
    EXPECT_EQ(KeReadyCount(KeGetReadyQueue()), 3);

    got = KeSchedule();
    EXPECT_TRUE(got == &t_high);
    EXPECT_EQ(got->State, ThreadStateRunning);

    KeTerminateThread(got);
    got = KeSchedule();
    EXPECT_TRUE(got == &t_mid);

    KeTerminateThread(got);
    got = KeSchedule();
    EXPECT_TRUE(got == &t_low);

    KiSystemShutdown();
    if (g_failures) { fprintf(stderr, "%d failures\n", g_failures); return 1; }
    printf("test_scheduler OK\n");
    return 0;
}
