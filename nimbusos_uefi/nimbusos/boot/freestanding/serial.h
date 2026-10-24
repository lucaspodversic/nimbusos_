/* NimbusOS — consola serie temprana (COM1 / 0x3F8).
 *
 * Usamos el UART 16550 clásico que QEMU emula con `-serial stdio`.
 * Sin interrupciones: polling de LSR.THRE / LSR.DR.
 */
#ifndef NIMBUS_FREESTANDING_SERIAL_H
#define NIMBUS_FREESTANDING_SERIAL_H

#include <stdint.h>
#include <stddef.h>

#define COM1_PORT 0x3F8u

void serial_init(void);
void serial_putc(char c);
void serial_write(const char *s);
void serial_write_n(const char *s, size_t n);
int  serial_ready(void);          /* 1 si hay byte pendiente */
int  serial_getc_nonblock(void);  /* -1 si vacío, sino 0..255 */

#endif
