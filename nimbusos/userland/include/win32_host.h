/* NimbusOS — cabeceras Win32 clean-room para tests en host.
 * No es el Windows SDK. Tipos y APIs mínimas reimplementadas.
 */
#ifndef NIMBUS_WIN32_HOST_H
#define NIMBUS_WIN32_HOST_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef uint32_t DWORD;
typedef unsigned int UINT;
typedef uint16_t WORD;
typedef uint8_t  BYTE;
typedef int32_t  LONG;
typedef uint32_t ULONG;
typedef int      BOOL;
typedef void*    HANDLE;
typedef void*    HMODULE;
typedef void*    HWND;
typedef void*    HDC;
typedef void*    HKEY;
typedef char*    LPSTR;
typedef const char* LPCSTR;
typedef wchar_t* LPWSTR;
typedef const wchar_t* LPCWSTR;
typedef size_t   SIZE_T;
typedef LONG     NTSTATUS;
typedef uintptr_t ULONG_PTR;

#define TRUE 1
#define FALSE 0
#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)
#define STATUS_SUCCESS ((NTSTATUS)0)
#define ERROR_SUCCESS 0
#define ERROR_FILE_NOT_FOUND 2
#define ERROR_INVALID_PARAMETER 87
#define ERROR_MORE_DATA 234

#define GENERIC_READ  0x80000000u
#define GENERIC_WRITE 0x40000000u
#define OPEN_EXISTING 3
#define CREATE_ALWAYS 2
#define FILE_ATTRIBUTE_NORMAL 0x80
#define MEM_COMMIT 0x1000
#define MEM_RESERVE 0x2000
#define PAGE_READWRITE 0x04
#define KEY_READ 0x20019
#define KEY_WRITE 0x20006
#define KEY_ALL_ACCESS 0xF003F
#define REG_SZ 1
#define REG_DWORD 4
#define REG_BINARY 3
#define HKEY_LOCAL_MACHINE ((HKEY)(uintptr_t)0x80000002u)
#define HKEY_CURRENT_USER  ((HKEY)(uintptr_t)0x80000001u)

#ifdef __cplusplus
extern "C" {
#endif

/* ---- ntdll / loader ---- */
typedef struct _NIMBUS_PE_IMAGE {
    const uint8_t* Base;
    size_t Size;
    uint32_t EntryRva;
    uint16_t NumSections;
    uint32_t ImageBase;
    int Valid;
} NIMBUS_PE_IMAGE;

int LdrParsePe(const uint8_t* Data, size_t Len, NIMBUS_PE_IMAGE* Out);
uint32_t LdrRvaToOffset(const uint8_t* Data, size_t Len, uint32_t Rva);

/* ---- kernel32 ---- */
HANDLE NimbusCreateFileA(LPCSTR path, DWORD access, DWORD creat);
BOOL   NimbusCloseHandle(HANDLE h);
BOOL   NimbusReadFile(HANDLE h, void* buf, DWORD n, DWORD* read);
BOOL   NimbusWriteFile(HANDLE h, const void* buf, DWORD n, DWORD* written);
void*  NimbusVirtualAlloc(void* addr, SIZE_T size, DWORD type, DWORD protect);
BOOL   NimbusVirtualFree(void* addr, SIZE_T size);
DWORD  NimbusGetLastError(void);
void   NimbusSetLastError(DWORD e);
HMODULE NimbusLoadLibraryA(LPCSTR name);
void*  NimbusGetProcAddress(HMODULE m, LPCSTR name);
DWORD  NimbusGetTickCount(void);
void   NimbusSleep(DWORD ms);

/* ---- advapi32 registry (host → CM hive) ---- */
LONG NimbusRegCreateKeyExA(HKEY root, LPCSTR sub, HKEY* out);
LONG NimbusRegOpenKeyExA(HKEY root, LPCSTR sub, HKEY* out);
LONG NimbusRegSetValueExA(HKEY k, LPCSTR name, DWORD type, const BYTE* data, DWORD cb);
LONG NimbusRegQueryValueExA(HKEY k, LPCSTR name, DWORD* type, BYTE* data, DWORD* cb);
LONG NimbusRegCloseKey(HKEY k);

/* ---- user32 minimal ---- */
HWND NimbusCreateWindowExA(LPCSTR cls, LPCSTR title, int x, int y, int w, int h);
BOOL NimbusDestroyWindow(HWND hwnd);
BOOL NimbusShowWindow(HWND hwnd, int cmd);
BOOL NimbusGetMessageA(void* msg);
BOOL NimbusPostMessageA(HWND hwnd, UINT msg, ULONG_PTR wp, ULONG_PTR lp);

#ifndef UINT
#endif

/* ---- gdi32 minimal ---- */
HDC  NimbusCreateCompatibleDC(HDC hdc);
BOOL NimbusDeleteDC(HDC hdc);
int  NimbusFillRectColor(HDC hdc, int x, int y, int w, int h, DWORD color);

/* ---- ws2_32 minimal ---- */
int NimbusWSAStartup(WORD ver);
int NimbusSocket(int af, int type, int proto);
int NimbusClosesocket(int s);

#ifdef __cplusplus
}
#endif
#endif
