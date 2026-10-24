/* advapi32 Reg* — host: hive en memoria (espejo del CM del kernel). */
#include "../../include/win32_host.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_KEYS 256
#define MAX_VALS 64

typedef struct {
    char name[64];
    DWORD type;
    BYTE data[256];
    DWORD cb;
} VAL;
typedef struct KeyNode {
    char name[128];
    struct KeyNode* children[32];
    int nchild;
    VAL vals[MAX_VALS];
    int nval;
    int used;
} KeyNode;

static KeyNode g_HKLM;
static KeyNode g_HKCU;
static int g_Init;

static void init_roots(void) {
    if (g_Init) return;
    memset(&g_HKLM, 0, sizeof(g_HKLM));
    memset(&g_HKCU, 0, sizeof(g_HKCU));
    strncpy(g_HKLM.name, "HKLM", sizeof(g_HKLM.name)-1);
    strncpy(g_HKCU.name, "HKCU", sizeof(g_HKCU.name)-1);
    g_HKLM.used = g_HKCU.used = 1;
    g_Init = 1;
}

static KeyNode* root_of(HKEY r) {
    init_roots();
    if (r == HKEY_LOCAL_MACHINE) return &g_HKLM;
    if (r == HKEY_CURRENT_USER) return &g_HKCU;
    return (KeyNode*)r;
}

static KeyNode* find_child(KeyNode* p, const char* name) {
    int i; for (i = 0; i < p->nchild; i++)
        if (strcmp(p->children[i]->name, name) == 0) return p->children[i];
    return NULL;
}

static KeyNode* walk(KeyNode* root, LPCSTR sub, int create) {
    char buf[256]; char* tok; char* save = NULL; KeyNode* cur = root;
    if (!sub || !*sub) return root;
    strncpy(buf, sub, sizeof(buf)-1); buf[sizeof(buf)-1] = 0;
    for (tok = strtok_r(buf, "\\/", &save); tok; tok = strtok_r(NULL, "\\/", &save)) {
        KeyNode* ch = find_child(cur, tok);
        if (!ch) {
            if (!create || cur->nchild >= 32) return NULL;
            ch = (KeyNode*)calloc(1, sizeof(KeyNode));
            if (!ch) return NULL;
            strncpy(ch->name, tok, sizeof(ch->name)-1);
            ch->used = 1;
            cur->children[cur->nchild++] = ch;
        }
        cur = ch;
    }
    return cur;
}

LONG NimbusRegCreateKeyExA(HKEY root, LPCSTR sub, HKEY* out) {
    KeyNode* k; if (!out) return ERROR_INVALID_PARAMETER;
    k = walk(root_of(root), sub, 1);
    if (!k) return ERROR_INVALID_PARAMETER;
    *out = (HKEY)k; return ERROR_SUCCESS;
}
LONG NimbusRegOpenKeyExA(HKEY root, LPCSTR sub, HKEY* out) {
    KeyNode* k; if (!out) return ERROR_INVALID_PARAMETER;
    k = walk(root_of(root), sub, 0);
    if (!k) return ERROR_FILE_NOT_FOUND;
    *out = (HKEY)k; return ERROR_SUCCESS;
}
LONG NimbusRegSetValueExA(HKEY k, LPCSTR name, DWORD type, const BYTE* data, DWORD cb) {
    KeyNode* node = (KeyNode*)k; int i;
    if (!node || !name || cb > 256) return ERROR_INVALID_PARAMETER;
    for (i = 0; i < node->nval; i++) {
        if (strcmp(node->vals[i].name, name) == 0) {
            node->vals[i].type = type; node->vals[i].cb = cb;
            memcpy(node->vals[i].data, data, cb); return ERROR_SUCCESS;
        }
    }
    if (node->nval >= MAX_VALS) return ERROR_INVALID_PARAMETER;
    strncpy(node->vals[node->nval].name, name, 63);
    node->vals[node->nval].type = type;
    node->vals[node->nval].cb = cb;
    memcpy(node->vals[node->nval].data, data, cb);
    node->nval++;
    return ERROR_SUCCESS;
}
LONG NimbusRegQueryValueExA(HKEY k, LPCSTR name, DWORD* type, BYTE* data, DWORD* cb) {
    KeyNode* node = (KeyNode*)k; int i;
    if (!node || !name || !cb) return ERROR_INVALID_PARAMETER;
    for (i = 0; i < node->nval; i++) {
        if (strcmp(node->vals[i].name, name) == 0) {
            if (type) *type = node->vals[i].type;
            if (!data) { *cb = node->vals[i].cb; return ERROR_SUCCESS; }
            if (*cb < node->vals[i].cb) { *cb = node->vals[i].cb; return ERROR_MORE_DATA; }
            memcpy(data, node->vals[i].data, node->vals[i].cb);
            *cb = node->vals[i].cb;
            return ERROR_SUCCESS;
        }
    }
    return ERROR_FILE_NOT_FOUND;
}
LONG NimbusRegCloseKey(HKEY k) { (void)k; return ERROR_SUCCESS; }
