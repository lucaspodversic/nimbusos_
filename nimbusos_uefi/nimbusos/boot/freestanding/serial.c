/* NimbusOS — driver serie freestanding (COM1).
 *
 * Comentarios detallados en español. Sin libc: inb/outb vía inline asm.
 */
#include "serial.h"

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void serial_init(void) {
    /* Deshabilitar IRQs del UART */
    outb(COM1_PORT + 1, 0x00);
    /* DLAB on → divisor baud */
    outb(COM1_PORT + 3, 0x80);
    /* 115200 / 1 = 115200 baud (divisor 1) — QEMU lo acepta bien */
    outb(COM1_PORT + 0, 0x01);
    outb(COM1_PORT + 1, 0x00);
    /* 8N1, DLAB off */
    outb(COM1_PORT + 3, 0x03);
    /* FIFO: enable, clear, 14-byte threshold */
    outb(COM1_PORT + 2, 0xC7);
    /* IRQs off, RTS/DSR set */
    outb(COM1_PORT + 4, 0x0B);
}

void serial_putc(char c) {
    if (c == '\n')
        serial_putc('\r');
    /* Esperar Transmitter Holding Register Empty (bit 5 de LSR) */
    while ((inb(COM1_PORT + 5) & 0x20) == 0)
        ;
    outb(COM1_PORT, (uint8_t)c);
}

void serial_write(const char *s) {
    if (!s) return;
    while (*s)
        serial_putc(*s++);
}

void serial_write_n(const char *s, size_t n) {
    for (size_t i = 0; i < n; i++)
        serial_putc(s[i]);
}

int serial_ready(void) {
    return (inb(COM1_PORT + 5) & 0x01) != 0;
}

int serial_getc_nonblock(void) {
    if (!serial_ready())
        return -1;
    return (int)inb(COM1_PORT);
}
