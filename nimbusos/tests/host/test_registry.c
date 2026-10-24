/* Test: hive round-trip (crear claves/valores, serializar, deserializar). */
#include "test_common.h"
#include "cm.h"

int main(void) {
    PCM_HIVE hive, hive2 = NULL;
    ULONG dword = 0xDEADBEEF, out = 0, sz;
    CM_VALUE_TYPE ty;
    UCHAR ser[256 * 1024];
    SIZE_T ser_sz = 0;
    NTSTATUS st;

    EXPECT_STATUS(KiSystemStartup(), STATUS_SUCCESS);
    hive = CmGetSystemHive();
    EXPECT_TRUE(hive != NULL);

    EXPECT_STATUS(CmCreateKey(hive, "\\Machine\\Software\\Nimbus"), STATUS_SUCCESS);
    EXPECT_STATUS(CmSetValue(hive, "\\Machine\\Software\\Nimbus", "Build",
                             CmTypeDword, &dword, sizeof(dword)), STATUS_SUCCESS);

    sz = sizeof(out);
    EXPECT_STATUS(CmQueryValue(hive, "\\Machine\\Software\\Nimbus", "Build",
                               &ty, &out, &sz), STATUS_SUCCESS);
    EXPECT_EQ(ty, CmTypeDword);
    EXPECT_EQ(out, 0xDEADBEEF);

    EXPECT_STATUS(CmCreateSnapshot(hive, "snap1"), STATUS_SUCCESS);

    st = CmSerializeHive(hive, ser, sizeof(ser), &ser_sz);
    EXPECT_STATUS(st, STATUS_SUCCESS);
    EXPECT_TRUE(ser_sz > 0);

    st = CmDeserializeHive(ser, ser_sz, &hive2);
    EXPECT_STATUS(st, STATUS_SUCCESS);
    EXPECT_TRUE(hive2 != NULL);

    out = 0; sz = sizeof(out);
    EXPECT_STATUS(CmQueryValue(hive2, "\\Machine\\Software\\Nimbus", "Build",
                               &ty, &out, &sz), STATUS_SUCCESS);
    EXPECT_EQ(out, 0xDEADBEEF);

    KiSystemShutdown();
    if (g_failures) { fprintf(stderr, "%d failures\n", g_failures); return 1; }
    printf("test_registry OK\n");
    return 0;
}
