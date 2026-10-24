/* NimbusOS — tipos básicos NT-compatibles (clean-room).
 * Sin código de Microsoft. Definiciones derivadas de documentación pública
 * de la API Win32/NT y del formato PE/COFF.
 *
 * Con NIMBUS_HOST=1 se compila como biblioteca en Linux para tests unitarios.
 */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Tipos de ancho fijo estilo NT ---- */
typedef void            VOID;
typedef void*           PVOID;
typedef const void*     PCVOID;
typedef uint8_t         UCHAR, BOOLEAN, BYTE;
typedef char            CHAR;
typedef uint16_t        USHORT, WCHAR;
typedef int16_t         SHORT;
typedef uint32_t        ULONG, DWORD, NTSTATUS, ACCESS_MASK;
typedef int32_t         LONG;
typedef uint64_t        ULONGLONG, ULONG64, SIZE_T, ULONG_PTR;
typedef int64_t         LONGLONG, LONG64, SSIZE_T, LONG_PTR;
typedef int             INT;
typedef unsigned        UINT;

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

/* Handles opacos (valor índice en tabla de handles del proceso). */
typedef void* HANDLE;
#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)
#define NT_SUCCESS(s) (((NTSTATUS)(s)) >= 0)

/* Alineación de página (4 KiB) — clave para MM y zram. */
#define PAGE_SIZE       4096u
#define PAGE_SHIFT      12u

/* Macro de conteo de elementos. */
#define NIMBUS_ARRAY_COUNT(a) (sizeof(a) / sizeof((a)[0]))

/* Atributos de sección / protección de página (subconjunto documentado). */
#define PAGE_NOACCESS          0x01u
#define PAGE_READONLY          0x02u
#define PAGE_READWRITE         0x04u
#define PAGE_EXECUTE           0x10u
#define PAGE_EXECUTE_READ      0x20u
#define PAGE_EXECUTE_READWRITE 0x40u

/* Códigos NTSTATUS mínimos usados en el núcleo. */
#define STATUS_SUCCESS                 ((NTSTATUS)0x00000000u)
#define STATUS_UNSUCCESSFUL            ((NTSTATUS)0xC0000001u)
#define STATUS_NOT_IMPLEMENTED         ((NTSTATUS)0xC0000002u)
#define STATUS_INVALID_PARAMETER       ((NTSTATUS)0xC000000Du)
#define STATUS_NO_MEMORY               ((NTSTATUS)0xC0000017u)
#define STATUS_ACCESS_DENIED           ((NTSTATUS)0xC0000022u)
#define STATUS_BUFFER_TOO_SMALL        ((NTSTATUS)0xC0000023u)
#define STATUS_OBJECT_NAME_NOT_FOUND   ((NTSTATUS)0xC0000034u)
#define STATUS_OBJECT_NAME_COLLISION   ((NTSTATUS)0xC0000035u)
#define STATUS_INSUFFICIENT_RESOURCES  ((NTSTATUS)0xC000009Au)
#define STATUS_INVALID_IMAGE_FORMAT    ((NTSTATUS)0xC000007Bu)
#define STATUS_NOT_FOUND               ((NTSTATUS)0xC0000225u)
#define STATUS_ALREADY_COMPLETE        ((NTSTATUS)0x00000000u)

/* Identificador de componente para el presupuesto de memoria. */
typedef enum _NIMBUS_COMPONENT_ID {
    NIMBUS_COMP_KERNEL = 0,
    NIMBUS_COMP_CACHE,
    NIMBUS_COMP_REGISTRY,
    NIMBUS_COMP_SERVICES,
    NIMBUS_COMP_WINDOW_SERVER,
    NIMBUS_COMP_SHELL,
    NIMBUS_COMP_AUDIO_NET,
    NIMBUS_COMP_SECURITY,
    NIMBUS_COMP_GPU_BUFFERS,
    NIMBUS_COMP_COUNT
} NIMBUS_COMPONENT_ID;

#ifdef __cplusplus
}
#endif
