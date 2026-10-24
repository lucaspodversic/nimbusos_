/* NimbusOS — printf mínimo sobre serial (solo %s %d %u %x %p %% ). */
#ifndef NIMBUS_KPRINT_H
#define NIMBUS_KPRINT_H

void kprint(const char *fmt, ...);
void kputs(const char *s);

#endif
