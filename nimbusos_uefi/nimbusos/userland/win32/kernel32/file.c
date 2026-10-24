/* kernel32 CreateFile/Read/Write — host: POSIX files. */
#include "../../include/win32_host.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static DWORD g_LastError;

void NimbusSetLastError(DWORD e) { g_LastError = e; }
DWORD NimbusGetLastError(void) { return g_LastError; }

typedef struct { FILE* fp; char path[260]; } FILE_OBJ;

HANDLE NimbusCreateFileA(LPCSTR path, DWORD access, DWORD creat) {
    FILE_OBJ* o; const char* mode = "rb";
    if (!path) { NimbusSetLastError(ERROR_INVALID_PARAMETER); return INVALID_HANDLE_VALUE; }
    if (creat == CREATE_ALWAYS) mode = (access & GENERIC_WRITE) ? "wb+" : "wb";
    else if (access & GENERIC_WRITE) mode = "rb+";
    o = (FILE_OBJ*)calloc(1, sizeof(*o));
    if (!o) return INVALID_HANDLE_VALUE;
    o->fp = fopen(path, mode);
    if (!o->fp && (access & GENERIC_WRITE)) o->fp = fopen(path, "wb+");
    if (!o->fp) { free(o); NimbusSetLastError(ERROR_FILE_NOT_FOUND); return INVALID_HANDLE_VALUE; }
    strncpy(o->path, path, sizeof(o->path)-1);
    return (HANDLE)o;
}
BOOL NimbusCloseHandle(HANDLE h) {
    FILE_OBJ* o = (FILE_OBJ*)h;
    if (!o || o == INVALID_HANDLE_VALUE) return FALSE;
    if (o->fp) fclose(o->fp);
    free(o); return TRUE;
}
BOOL NimbusReadFile(HANDLE h, void* buf, DWORD n, DWORD* read) {
    FILE_OBJ* o = (FILE_OBJ*)h; size_t r;
    if (!o || !o->fp || !buf) return FALSE;
    r = fread(buf, 1, n, o->fp);
    if (read) *read = (DWORD)r;
    return TRUE;
}
BOOL NimbusWriteFile(HANDLE h, const void* buf, DWORD n, DWORD* written) {
    FILE_OBJ* o = (FILE_OBJ*)h; size_t w;
    if (!o || !o->fp || !buf) return FALSE;
    w = fwrite(buf, 1, n, o->fp);
    if (written) *written = (DWORD)w;
    return w == n;
}
