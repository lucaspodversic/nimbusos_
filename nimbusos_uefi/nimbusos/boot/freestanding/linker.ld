/* NimbusOS — script de enlace freestanding (ELF64 Multiboot2).
 *
 * El encabezado Multiboot2 debe quedar en los primeros 32 KiB de la imagen
 * (requisito del especificador Multiboot2). Cargamos el kernel en 1 MiB
 * físico, identidad-mapeado, para poder arrancar desde GRUB sin paginación
 * previa compleja.
 */
OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)

SECTIONS
{
    . = 1M;

    .multiboot2 ALIGN(8) : {
        KEEP(*(.multiboot2))
    }

    .text ALIGN(4K) : {
        *(.text.boot)
        *(.text .text.*)
    }

    .rodata ALIGN(4K) : {
        *(.rodata .rodata.*)
    }

    .data ALIGN(4K) : {
        *(.data .data.*)
    }

    .bss ALIGN(4K) : {
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        /* Pila del BSP (16 KiB) DENTRO de .bss: así el segmento PT_LOAD la
         * cubre (p_memsz) y el cargador UEFI (BOOTX64.EFI) reserva y pone a
         * cero esa memoria. GRUB/Multiboot2 también la respeta. */
        . = ALIGN(16);
        __stack_bottom = .;
        . += 16K;
        __stack_top = .;
        __bss_end = .;
    }

    /DISCARD/ : {
        *(.eh_frame*)
        *(.note*)
        *(.comment*)
    }
}
