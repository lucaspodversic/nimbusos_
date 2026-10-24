/* NimbusOS — contrato de traspaso BOOTX64.EFI → kernel (ruta UEFI).
 *
 * Este encabezado lo comparten el cargador UEFI (boot/uefi/app) y el kernel
 * freestanding. Solo usa tipos de <stdint.h> para compilar en ambos lados.
 *
 * El cargador copia el mapa de memoria final (el que entregó a
 * ExitBootServices) a memoria EfiLoaderData, que el kernel puede leer porque
 * todo está identidad-mapeado por debajo de 4 GiB.
 */
#ifndef NIMBUS_BOOTINFO_H
#define NIMBUS_BOOTINFO_H

#include <stdint.h>

#define NIMBUS_UEFI_MAGIC        0x4E554546u   /* "NUEF" */
#define NIMBUS_BOOTINFO_VERSION  1u

/* Tipos de memoria UEFI relevantes (UEFI spec, EFI_MEMORY_TYPE). */
#define NIMBUS_EFI_LOADER_CODE         1u
#define NIMBUS_EFI_LOADER_DATA         2u
#define NIMBUS_EFI_BOOT_SERVICES_CODE  3u
#define NIMBUS_EFI_BOOT_SERVICES_DATA  4u
#define NIMBUS_EFI_CONVENTIONAL        7u

/* Descriptor de memoria UEFI (prefijo estable; el tamaño real del
 * descriptor lo da MemMapDescSize, que puede ser mayor que este struct). */
typedef struct {
    uint32_t Type;
    uint32_t Pad;
    uint64_t PhysicalStart;
    uint64_t VirtualStart;
    uint64_t NumberOfPages;
    uint64_t Attribute;
} NIMBUS_EFI_MEMDESC;

typedef struct {
    uint32_t Magic;            /* NIMBUS_UEFI_MAGIC */
    uint32_t Version;          /* NIMBUS_BOOTINFO_VERSION */
    uint64_t MemMap;           /* dirección física del primer descriptor */
    uint64_t MemMapSize;       /* bytes totales del mapa */
    uint64_t MemMapDescSize;   /* bytes por descriptor */
    uint64_t KernelBase;       /* dirección física donde se cargó kernel.elf */
    uint64_t KernelSize;       /* bytes reservados para el kernel */
    uint64_t AcpiRsdp;         /* RSDP (ACPI 2.0) si el firmware lo publica, o 0 */
} NIMBUS_UEFI_BOOTINFO;

#endif
