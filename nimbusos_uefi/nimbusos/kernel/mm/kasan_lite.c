/* NimbusOS — KASAN-lite: marca de envenenamiento en sombra (host). */
#include "mm.h"
#include "../host/host_compat.h"

#define KASAN_SHADOW_SIZE (1u << 20)
static UCHAR g_Shadow[KASAN_SHADOW_SIZE];

void MmKasanLitePoison(void* P, SIZE_T N) {
    SIZE_T i; UCHAR* p = (UCHAR*)P;
    for (i = 0; i < N && i < KASAN_SHADOW_SIZE; i++)
        g_Shadow[((SIZE_T)(p + i)) % KASAN_SHADOW_SIZE] = 0xFF;
}
void MmKasanLiteUnpoison(void* P, SIZE_T N) {
    SIZE_T i; UCHAR* p = (UCHAR*)P;
    for (i = 0; i < N && i < KASAN_SHADOW_SIZE; i++)
        g_Shadow[((SIZE_T)(p + i)) % KASAN_SHADOW_SIZE] = 0;
}
