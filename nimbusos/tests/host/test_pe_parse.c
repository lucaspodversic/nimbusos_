/* Test: parseo y mapeo de un PE x64 mínimo fabricado. */
#include "test_common.h"
#include "ps.h"

int main(void) {
    UCHAR buf[4096];
    SIZE_T n;
    PE_IMAGE_INFO info;
    PVOID base = NULL;
    NTSTATUS st;

    EXPECT_STATUS(KiSystemStartup(), STATUS_SUCCESS);

    n = PeCraftMinimalX64(buf, sizeof(buf), 0x1000);
    EXPECT_TRUE(n > 0);

    st = PeParseImage(buf, n, &info);
    EXPECT_STATUS(st, STATUS_SUCCESS);
    EXPECT_EQ(info.Machine, 0x8664);
    EXPECT_EQ(info.NumberOfSections, 1);
    EXPECT_EQ(info.AddressOfEntryPoint, 0x1000);
    EXPECT_TRUE(info.SizeOfImage >= 0x2000);

    st = PeMapImage(&info, &base);
    EXPECT_STATUS(st, STATUS_SUCCESS);
    EXPECT_TRUE(base != NULL);
    /* El byte en entry RVA debe ser 0xC3 (ret) */
    EXPECT_EQ(((UCHAR*)base)[0x1000], 0xC3);

    free(base);
    KiSystemShutdown();
    if (g_failures) { fprintf(stderr, "%d failures\n", g_failures); return 1; }
    printf("test_pe_parse OK\n");
    return 0;
}
