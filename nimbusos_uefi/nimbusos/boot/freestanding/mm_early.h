/* NimbusOS — subconjunto MM temprano: allocador bump sobre un heap estático. */
#ifndef NIMBUS_MM_EARLY_H
#define NIMBUS_MM_EARLY_H

#include <stddef.h>
#include <stdint.h>

void   mm_early_init(void);
void  *mm_early_alloc(size_t size);
size_t mm_early_used(void);
size_t mm_early_total(void);

#endif
