/* NimbusOS — stubs freestanding cuando NO hay NIMBUS_HOST. */
#ifndef NIMBUS_HOST
#include "host_compat.h"

/* En bare-metal real estos vendrían del boot heap; aquí son stubs débiles. */
static UCHAR g_EarlyHeap[256 * 1024];
static SIZE_T g_EarlyUsed;

void* nimbus_host_malloc(size_t sz) {
    void* p;
    sz = (sz + 15) & ~(size_t)15;
    if (g_EarlyUsed + sz > sizeof(g_EarlyHeap)) return NULL;
    p = &g_EarlyHeap[g_EarlyUsed];
    g_EarlyUsed += sz;
    return p;
}
void* nimbus_host_calloc(size_t n, size_t sz) {
    size_t t = n * sz; void* p = nimbus_host_malloc(t);
    if (p) { size_t i; for (i = 0; i < t; i++) ((UCHAR*)p)[i] = 0; }
    return p;
}
void* nimbus_host_realloc(void* p, size_t sz) { (void)p; return nimbus_host_malloc(sz); }
void  nimbus_host_free(void* p) { (void)p; }
void* nimbus_host_memcpy(void* d, const void* s, size_t n) {
    UCHAR* D=d; const UCHAR* S=s; while(n--) *D++=*S++; return d;
}
void* nimbus_host_memset(void* d, int c, size_t n) {
    UCHAR* D=d; while(n--) *D++=(UCHAR)c; return d;
}
int nimbus_host_memcmp(const void* a, const void* b, size_t n) {
    const UCHAR*A=a,*B=b; while(n--){ if(*A!=*B) return *A-*B; A++; B++; } return 0;
}
size_t nimbus_host_strlen(const char* s){ size_t n=0; while(s[n]) n++; return n; }
int nimbus_host_strcmp(const char* a, const char* b){
    while(*a && *a==*b){a++;b++;} return (UCHAR)*a-(UCHAR)*b;
}
char* nimbus_host_strncpy(char* d, const char* s, size_t n){
    size_t i; for(i=0;i<n && s[i];i++) d[i]=s[i]; for(;i<n;i++) d[i]=0; return d;
}
int nimbus_host_printf(const char* fmt, ...){ (void)fmt; return 0; }
#endif
