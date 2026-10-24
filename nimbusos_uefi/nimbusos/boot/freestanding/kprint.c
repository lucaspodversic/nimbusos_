/* NimbusOS — formateo mínimo freestanding → serial. */
#include "kprint.h"
#include "serial.h"
#include <stdarg.h>
#include <stdint.h>

extern size_t strlen(const char *s);
/* size_t from stddef via serial.h path — include explicitly */
#include <stddef.h>

static void put_uint(unsigned long long v, unsigned base, int width, char pad) {
    char buf[32];
    const char *digits = "0123456789abcdef";
    int i = 0;
    if (v == 0) buf[i++] = '0';
    else while (v) { buf[i++] = digits[v % base]; v /= base; }
    while (i < width) buf[i++] = pad;
    while (i--) serial_putc(buf[i]);
}

void kputs(const char *s) {
    serial_write(s);
    serial_putc('\n');
}

void kprint(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    for (const char *p = fmt; *p; p++) {
        if (*p != '%') { serial_putc(*p); continue; }
        p++;
        int width = 0;
        char pad = ' ';
        if (*p == '0') { pad = '0'; p++; }
        while (*p >= '0' && *p <= '9') { width = width * 10 + (*p - '0'); p++; }
        switch (*p) {
        case 's': {
            const char *s = va_arg(ap, const char *);
            serial_write(s ? s : "(null)");
            break;
        }
        case 'd': {
            int v = va_arg(ap, int);
            if (v < 0) { serial_putc('-'); put_uint((unsigned)-v, 10, width, pad); }
            else put_uint((unsigned)v, 10, width, pad);
            break;
        }
        case 'u':
            put_uint(va_arg(ap, unsigned), 10, width, pad);
            break;
        case 'x':
            put_uint(va_arg(ap, unsigned), 16, width, pad);
            break;
        case 'p':
            serial_write("0x");
            put_uint((unsigned long long)(uintptr_t)va_arg(ap, void *), 16, 16, '0');
            break;
        case '%':
            serial_putc('%');
            break;
        default:
            serial_putc('%');
            serial_putc(*p);
            break;
        }
    }
    va_end(ap);
}
