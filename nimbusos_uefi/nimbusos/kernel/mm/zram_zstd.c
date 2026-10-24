/* NimbusOS — stub Zstd: reutiliza el compresor LZ4 embebido como fallback.
 * En producción se enlazaría libzstd; aquí mantenemos dependencia cero.
 */
#include "mm.h"

SIZE_T MmZstdCompressStub(const void* Src, SIZE_T SrcLen, void* Dst, SIZE_T DstCap) {
    return MmLz4Compress(Src, SrcLen, Dst, DstCap);
}
SIZE_T MmZstdDecompressStub(const void* Src, SIZE_T SrcLen, void* Dst, SIZE_T DstCap) {
    return MmLz4Decompress(Src, SrcLen, Dst, DstCap);
}
