/* NimbusOS — kmain64: punto de entrada C tras Multiboot2 → long mode.
 *
 * Secuencia de arranque freestanding (ISO QEMU):
 *   1. Inicializar UART COM1 (serial)
 *   2. Imprimir banner "NimbusOS"
 *   3. Validar magic Multiboot2
 *   4. Inicializar subconjunto Mm (bump heap)
 *   5. Entrar al shell serie (o idle respondiendo en serial)
 *
 * Compilado con -ffreestanding -nostdlib. Sin NIMBUS_HOST.
 */
#include "serial.h"
#include "kprint.h"
#include "mm_early.h"
#include "nimbus_bootinfo.h"
#include <stdint.h>

#define MULTIBOOT2_BOOTLOADER_MAGIC 0x36d76289u

extern void nimbus_shell_run(void);

/* Ruta de arranque (la muestra el comando `about` del shell). */
const char *g_nimbus_boot_path = "desconocida";

/* Subconjunto Mm para UEFI: recorre el mapa de memoria que entregó el
 * firmware y suma la RAM utilizable tras ExitBootServices (Conventional +
 * BootServices Code/Data + Loader Code/Data). Es la base que usaría el
 * allocator físico real (kernel/mm/physical_alloc.c) en fases posteriores. */
static void mm_uefi_memmap_summary(const NIMBUS_UEFI_BOOTINFO *bi) {
    if (!bi || bi->Magic != NIMBUS_UEFI_MAGIC || !bi->MemMap || !bi->MemMapDescSize) {
        kputs("UEFI: bootinfo invalido");
        return;
    }
    uint64_t usable_pages = 0, conventional_pages = 0;
    uint64_t n = bi->MemMapSize / bi->MemMapDescSize;
    for (uint64_t i = 0; i < n; i++) {
        const NIMBUS_EFI_MEMDESC *d = (const NIMBUS_EFI_MEMDESC *)
            (uintptr_t)(bi->MemMap + i * bi->MemMapDescSize);
        switch (d->Type) {
        case NIMBUS_EFI_CONVENTIONAL:
            conventional_pages += d->NumberOfPages;
            /* fallthrough */
        case NIMBUS_EFI_LOADER_CODE:
        case NIMBUS_EFI_LOADER_DATA:
        case NIMBUS_EFI_BOOT_SERVICES_CODE:
        case NIMBUS_EFI_BOOT_SERVICES_DATA:
            usable_pages += d->NumberOfPages;
            break;
        default:
            break;
        }
    }
    kprint("UEFI memmap: %u descriptores, utilizable=%u MiB (conventional=%u MiB)\n",
           (unsigned)n, (unsigned)(usable_pages / 256u), (unsigned)(conventional_pages / 256u));
    kprint("UEFI: kernel cargado en %p (%u KiB)\n",
           (void *)(uintptr_t)bi->KernelBase, (unsigned)(bi->KernelSize / 1024u));
}

/* Demostración mínima de Mm: reservar un bloque y escribir un marcador. */
static void mm_smoke_test(void) {
    void *p = mm_early_alloc(64);
    if (!p) {
        kputs("Mm: alloc fallo");
        return;
    }
    volatile uint32_t *w = (volatile uint32_t *)p;
    w[0] = 0x4E494D42u; /* 'NIMB' little-endian-ish tag */
    kprint("Mm: smoke OK ptr=%p used=%u\n", p, (unsigned)mm_early_used());
}

void kmain64(uint32_t magic, uint64_t mb_info) {
    serial_init();

    kputs("");
    kputs("============================================");
    kputs("            NimbusOS");
    kputs("   clean-room OS  |  freestanding boot");
    kputs("============================================");

    if (magic == MULTIBOOT2_BOOTLOADER_MAGIC) {
        g_nimbus_boot_path = "Multiboot2 + GRUB (BIOS)";
        kputs("Multiboot2: OK");
    } else if (magic == NIMBUS_UEFI_MAGIC) {
        g_nimbus_boot_path = "UEFI (BOOTX64.EFI, ExitBootServices)";
        kputs("Boot: UEFI OK (BOOTX64.EFI -> ExitBootServices -> long mode handoff)");
        mm_uefi_memmap_summary((const NIMBUS_UEFI_BOOTINFO *)(uintptr_t)mb_info);
    } else {
        kprint("AVISO: magic de arranque inesperado: 0x%x\n", magic);
    }

    mm_early_init();
    kprint("Mm early init: heap %u bytes\n", (unsigned)mm_early_total());
    mm_smoke_test();

    kputs("NimbusOS: kernel freestanding listo");
    kputs("NimbusOS: entrando a shell serie");

    /* Shell bloqueante; el test QEMU solo necesita ver el banner arriba. */
    nimbus_shell_run();
}
