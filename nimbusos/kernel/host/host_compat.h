/* NimbusOS — capa de compatibilidad para compilar el kernel en Linux (host).
 *
 * Con -DNIMBUS_HOST=1 el kernel se enlaza como biblioteca estática y se
 * ejecutan tests unitarios sin QEMU. En bare-metal (sin NIMBUS_HOST) se
 * usan stubs freestanding (sin libc completa).
 */
#pragma once

#ifdef NIMBUS_HOST

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <time.h>

#define nimbus_host_malloc(sz)      malloc(sz)
#define nimbus_host_calloc(n, sz)   calloc(n, sz)
#define nimbus_host_realloc(p, sz)  realloc(p, sz)
#define nimbus_host_free(p)         free(p)
#define nimbus_host_memcpy          memcpy
#define nimbus_host_memset          memset
#define nimbus_host_memcmp          memcmp
#define nimbus_host_strlen          strlen
#define nimbus_host_strcmp          strcmp
#define nimbus_host_strncpy         strncpy
#define nimbus_host_printf          printf

#else /* freestanding bare-metal stubs */

#include <stddef.h>
#include <stdint.h>

void* nimbus_host_malloc(size_t sz);
void* nimbus_host_calloc(size_t n, size_t sz);
void* nimbus_host_realloc(void* p, size_t sz);
void  nimbus_host_free(void* p);
void* nimbus_host_memcpy(void* d, const void* s, size_t n);
void* nimbus_host_memset(void* d, int c, size_t n);
int   nimbus_host_memcmp(const void* a, const void* b, size_t n);
size_t nimbus_host_strlen(const char* s);
int   nimbus_host_strcmp(const char* a, const char* b);
char* nimbus_host_strncpy(char* d, const char* s, size_t n);
int   nimbus_host_printf(const char* fmt, ...);

#endif
