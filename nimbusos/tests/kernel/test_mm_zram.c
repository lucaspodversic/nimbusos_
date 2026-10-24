/* NimbusOS — test_mm_zram.c (clean-room).
 * Comentarios en español; ejecutable de humo.
 */
/* Smoke: incluye API zram del kernel host. */
#include "../../kernel/include/mm.h"
#include <stdio.h>
extern NTSTATUS MmZramCompress(const void*, SIZE_T, void*, SIZE_T*);
int main(void){ printf("kernel zram smoke — usar tests/host/test_zram\n"); return 0; }
