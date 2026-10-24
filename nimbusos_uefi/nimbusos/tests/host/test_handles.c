/* Test: tabla de handles del Object Manager. */
#include "test_common.h"
#include "ob.h"

int main(void) {
    HANDLE_TABLE table;
    POBJECT_HEADER obj, got = NULL;
    HANDLE h = NULL;
    NTSTATUS st;

    EXPECT_STATUS(KiSystemStartup(), STATUS_SUCCESS);
    ObInitHandleTable(&table);

    obj = ObCreateObject(ObTypeEvent, "\\TestEvent", 16);
    EXPECT_TRUE(obj != NULL);
    EXPECT_EQ(obj->RefCount, 1);

    st = ObInsertHandle(&table, obj, GENERIC_ALL, &h);
    EXPECT_STATUS(st, STATUS_SUCCESS);
    EXPECT_TRUE(h != NULL);
    EXPECT_EQ(table.Count, 1);
    EXPECT_EQ(obj->HandleCount, 1);
    /* RefCount: create=1 + insert=1 => 2 */
    EXPECT_EQ(obj->RefCount, 2);

    st = ObReferenceObjectByHandle(&table, h, GENERIC_ALL, &got);
    EXPECT_STATUS(st, STATUS_SUCCESS);
    EXPECT_TRUE(got == obj);
    EXPECT_EQ(obj->RefCount, 3);
    ObDereferenceObject(got);

    st = ObCloseHandle(&table, h);
    EXPECT_STATUS(st, STATUS_SUCCESS);
    EXPECT_EQ(table.Count, 0);

    ObDereferenceObject(obj); /* última ref del create */

    KiSystemShutdown();
    if (g_failures) { fprintf(stderr, "%d failures\n", g_failures); return 1; }
    printf("test_handles OK\n");
    return 0;
}
