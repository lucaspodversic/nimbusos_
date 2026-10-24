/* Test: compresión/descompresión zram LZ4 de una página. */
#include "test_common.h"
#include "mm.h"

int main(void) {
    UCHAR page[PAGE_SIZE];
    UCHAR out[PAGE_SIZE];
    UCHAR comp[PAGE_SIZE];
    SIZE_T csz, dsz;
    ULONG pfn = 0;
    NTSTATUS st;

    EXPECT_STATUS(KiSystemStartup(), STATUS_SUCCESS);

    /* Página muy repetitiva → comprime bien. */
    memset(page, 0xAB, PAGE_SIZE);
    memcpy(page + 100, "NimbusOS-zram-test-pattern", 26);

    csz = MmLz4Compress(page, PAGE_SIZE, comp, sizeof(comp));
    EXPECT_TRUE(csz > 0);
    EXPECT_TRUE(csz < PAGE_SIZE);

    memset(out, 0, sizeof(out));
    dsz = MmLz4Decompress(comp, csz, out, sizeof(out));
    EXPECT_EQ(dsz, PAGE_SIZE);
    EXPECT_EQ(memcmp(page, out, PAGE_SIZE), 0);

    /* Vía zram slots */
    EXPECT_STATUS(MmAllocatePhysicalPages(1, &pfn), STATUS_SUCCESS);
    st = MmZramCompressPage(pfn, page);
    EXPECT_STATUS(st, STATUS_SUCCESS);
    EXPECT_TRUE(MmZramCompressedCount() >= 1);

    memset(out, 0, sizeof(out));
    EXPECT_STATUS(MmZramDecompressPage(pfn, out), STATUS_SUCCESS);
    EXPECT_EQ(memcmp(page, out, PAGE_SIZE), 0);

    MmFreePhysicalPages(pfn, 1);
    KiSystemShutdown();
    if (g_failures) { fprintf(stderr, "%d failures\n", g_failures); return 1; }
    printf("test_zram OK\n");
    return 0;
}
