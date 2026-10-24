/* NimbusOS — shell serie mínima (demo ISO / QEMU).
 *
 * Comandos:
 *   help / ?     — lista comandos
 *   about        — banner NimbusOS
 *   mm           — estado del bump allocator
 *   echo <txt>   — eco
 *   clear        — línea en blanco
 *   halt / quit  — detiene la CPU (hlt loop)
 *
 * Si no hay entrada, el bucle idle responde a teclas cuando llegan.
 */
#include "serial.h"
#include "kprint.h"
#include "mm_early.h"
#include <stddef.h>

/* Ruta de arranque detectada en kmain64 ("Multiboot2 + GRUB" o "UEFI"). */
extern const char *g_nimbus_boot_path;

extern int strcmp(const char *a, const char *b);
extern int strncmp(const char *a, const char *b, size_t n);
extern size_t strlen(const char *s);

#define LINE_MAX 128

static void cmd_help(void) {
    kputs("Comandos NimbusOS (serial):");
    kputs("  help | ?     mostrar esta ayuda");
    kputs("  about        informacion del sistema");
    kputs("  mm           memoria temprana (bump)");
    kputs("  echo <txt>   repetir texto");
    kputs("  clear        linea en blanco");
    kputs("  halt | quit  detener CPU");
}

static void cmd_about(void) {
    kputs("========================================");
    kputs("  NimbusOS  —  clean-room OS (x86_64)");
    kprint("  Boot: %s  |  Serial COM1\n", g_nimbus_boot_path);
    kputs("  No es Windows. Producto: NimbusOS.");
    kputs("========================================");
}

static void cmd_mm(void) {
    kprint("Mm early: used=%u / total=%u bytes\n",
           (unsigned)mm_early_used(), (unsigned)mm_early_total());
}

static void run_line(char *line) {
    /* trim leading spaces */
    while (*line == ' ' || *line == '\t') line++;
    if (!*line) return;

    if (strcmp(line, "help") == 0 || strcmp(line, "?") == 0) {
        cmd_help();
    } else if (strcmp(line, "about") == 0) {
        cmd_about();
    } else if (strcmp(line, "mm") == 0) {
        cmd_mm();
    } else if (strcmp(line, "clear") == 0) {
        kputs("");
    } else if (strcmp(line, "halt") == 0 || strcmp(line, "quit") == 0) {
        kputs("NimbusOS: halt");
        for (;;)
            __asm__ volatile ("hlt");
    } else if (strncmp(line, "echo ", 5) == 0) {
        kputs(line + 5);
    } else {
        kprint("desconocido: %s  (help para lista)\n", line);
    }
}

void nimbus_shell_run(void) {
    char line[LINE_MAX];
    size_t len = 0;

    kputs("NimbusOS shell lista. Escribe 'help'.");
    serial_write("nimbus> ");

    for (;;) {
        int c = serial_getc_nonblock();
        if (c < 0) {
            /* Idle: ceder ciclo (pause) sin apagar del todo — permite
             * que QEMU siga emulando y el test capture el banner. */
            __asm__ volatile ("pause");
            continue;
        }
        char ch = (char)c;
        if (ch == '\r' || ch == '\n') {
            serial_putc('\n');
            line[len] = '\0';
            run_line(line);
            len = 0;
            serial_write("nimbus> ");
            continue;
        }
        if (ch == 0x7f || ch == 0x08) { /* backspace */
            if (len > 0) {
                len--;
                serial_write("\b \b");
            }
            continue;
        }
        if (ch >= 32 && ch < 127 && len + 1 < LINE_MAX) {
            line[len++] = ch;
            serial_putc(ch);
        }
    }
}
