/* Test: parsea un PE x64 mínimo craftado con LdrParsePe. */
#include "win32_host.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Construye PE32+ mínimo en memoria. */
static size_t craft_pe(uint8_t* buf, size_t cap) {
    size_t o = 0;
    memset(buf, 0, cap);
    /* DOS */
    buf[0]='M'; buf[1]='Z';
    *(int32_t*)(buf+0x3C) = 0x80; /* e_lfanew */
    o = 0x80;
    /* PE sig */
    buf[o]= 'P'; buf[o+1]='E'; o += 4;
    /* COFF: Machine AMD64, 1 section, SizeOfOptionalHeader=0xF0 */
    *(uint16_t*)(buf+o) = 0x8664; o+=2;
    *(uint16_t*)(buf+o) = 1; o+=2; /* sections */
    o += 12; /* timestamps/symbols */
    *(uint16_t*)(buf+o) = 0xF0; o+=2; /* SizeOfOptionalHeader */
    *(uint16_t*)(buf+o) = 0x22; o+=2; /* Characteristics */
    /* Optional PE32+ */
    *(uint16_t*)(buf+o) = 0x20B; o+=2; /* Magic */
    o += 14; /* linker + sizes placeholder */
    *(uint32_t*)(buf+0x80+24+16) = 0x1000; /* AddressOfEntryPoint at opt+16 */
    /* ImageBase at opt+24 for PE32+ */
    *(uint64_t*)(buf+0x80+24+24) = 0x140000000ULL;
    /* Section header after optional */
    {
        size_t sec = 0x80 + 24 + 0xF0;
        memcpy(buf+sec, ".text\0\0", 8);
        *(uint32_t*)(buf+sec+8) = 0x200;  /* VirtualSize */
        *(uint32_t*)(buf+sec+12) = 0x1000; /* VA */
        *(uint32_t*)(buf+sec+16) = 0x200; /* SizeOfRawData */
        *(uint32_t*)(buf+sec+20) = 0x200; /* PointerToRawData */
    }
    return 0x400;
}

int main(void) {
    uint8_t* pe = calloc(1, 0x800);
    NIMBUS_PE_IMAGE img;
    size_t n;
    if (!pe) return 1;
    n = craft_pe(pe, 0x800);
    if (!LdrParsePe(pe, n, &img)) {
        fprintf(stderr, "LdrParsePe failed\n");
        return 2;
    }
    if (!img.Valid || img.NumSections != 1) {
        fprintf(stderr, "unexpected: sections=%u valid=%d\n", img.NumSections, img.Valid);
        return 3;
    }
    printf("test_win32_pe OK entry=0x%x\n", img.EntryRva);
    free(pe);
    return 0;
}
