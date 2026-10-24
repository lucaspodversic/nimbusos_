/* NimbusOS — compresor LZ77 mínimo estilo LZ4 (clean-room, formato propio).
 *
 * Formato del bloque:
 *   [u32 original_size]
 *   repetir hasta consumir original_size:
 *     [u8 lit_len][literales…][u16 offset little-endian][u8 match_len]
 *   offset==0 && match_len==0 ⇒ solo literales (sin copia).
 *
 * Suficiente para páginas de 4 KiB en tests; no interoperable con lz4frame.
 */
#include "mm.h"
#include "../host/host_compat.h"

#define MIN_MATCH 4
#define MAX_LIT   255
#define MAX_MATCH 255
#define WINDOW    4096

SIZE_T MmLz4Compress(const void* Src, SIZE_T SrcLen, void* Dst, SIZE_T DstCap) {
    const UCHAR* s = (const UCHAR*)Src;
    UCHAR* d = (UCHAR*)Dst;
    SIZE_T ip = 0, op = 0;

    if (!Src || !Dst || DstCap < 8 || SrcLen == 0 || SrcLen > 0xFFFFFFFFu)
        return 0;

    /* cabecera: tamaño original */
    d[op++] = (UCHAR)(SrcLen & 0xFF);
    d[op++] = (UCHAR)((SrcLen >> 8) & 0xFF);
    d[op++] = (UCHAR)((SrcLen >> 16) & 0xFF);
    d[op++] = (UCHAR)((SrcLen >> 24) & 0xFF);

    while (ip < SrcLen) {
        SIZE_T lit_start = ip;
        SIZE_T best_off = 0, best_len = 0;

        /* Limitar literales pendientes a MAX_LIT forzando flush. */
        while (ip < SrcLen && (ip - lit_start) < MAX_LIT) {
            SIZE_T win_start = ip > WINDOW ? ip - WINDOW : 0;
            SIZE_T j, len;
            best_off = 0;
            best_len = 0;
            for (j = win_start; j + MIN_MATCH <= ip; j++) {
                len = 0;
                while (ip + len < SrcLen && len < MAX_MATCH && s[j + len] == s[ip + len])
                    len++;
                if (len >= MIN_MATCH && len > best_len) {
                    best_len = len;
                    best_off = ip - j;
                }
            }
            if (best_len >= MIN_MATCH)
                break;
            ip++;
        }

        {
            SIZE_T lit_len = ip - lit_start;
            SIZE_T need = 1 + lit_len + 2 + 1;
            if (op + need > DstCap)
                return 0;
            d[op++] = (UCHAR)lit_len;
            if (lit_len)
                nimbus_host_memcpy(d + op, s + lit_start, lit_len);
            op += lit_len;

            if (best_len >= MIN_MATCH) {
                d[op++] = (UCHAR)(best_off & 0xFF);
                d[op++] = (UCHAR)((best_off >> 8) & 0xFF);
                d[op++] = (UCHAR)best_len;
                ip += best_len;
            } else {
                /* fin de entrada o solo literales */
                d[op++] = 0;
                d[op++] = 0;
                d[op++] = 0;
            }
        }
    }
    return op;
}

SIZE_T MmLz4Decompress(const void* Src, SIZE_T SrcLen, void* Dst, SIZE_T DstCap) {
    const UCHAR* s = (const UCHAR*)Src;
    UCHAR* d = (UCHAR*)Dst;
    SIZE_T ip = 0, op = 0;
    SIZE_T original;

    if (!Src || !Dst || SrcLen < 4)
        return 0;

    original = (SIZE_T)s[0] | ((SIZE_T)s[1] << 8) | ((SIZE_T)s[2] << 16) | ((SIZE_T)s[3] << 24);
    ip = 4;
    if (original > DstCap)
        return 0;

    while (op < original) {
        UCHAR lit_len;
        USHORT off;
        UCHAR match_len;
        if (ip >= SrcLen)
            return 0;
        lit_len = s[ip++];
        if (ip + lit_len + 3 > SrcLen)
            return 0;
        if (op + lit_len > original)
            return 0;
        if (lit_len)
            nimbus_host_memcpy(d + op, s + ip, lit_len);
        ip += lit_len;
        op += lit_len;

        off = (USHORT)(s[ip] | (s[ip + 1] << 8));
        ip += 2;
        match_len = s[ip++];

        if (off == 0 && match_len == 0)
            continue;
        if (off == 0 || off > op)
            return 0;
        if (op + match_len > original)
            return 0;
        {
            SIZE_T k;
            SIZE_T src_pos = op - off;
            for (k = 0; k < match_len; k++)
                d[op + k] = d[src_pos + k];
            op += match_len;
        }
    }
    return (op == original) ? op : 0;
}
