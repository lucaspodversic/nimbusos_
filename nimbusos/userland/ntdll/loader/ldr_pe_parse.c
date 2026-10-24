/* NimbusOS ntdll — parser PE/COFF x64 (clean-room, host-testeable).
 * Soporta DOS stub, PE signature, COFF, optional header PE32+, secciones.
 */
#include "../../include/win32_host.h"
#include <string.h>

#pragma pack(push, 1)
typedef struct { uint16_t e_magic; uint8_t pad[58]; int32_t e_lfanew; } DOS_HDR;
typedef struct {
    uint32_t Sig; uint16_t Machine, NumberOfSections;
    uint32_t TimeDateStamp, PointerToSymbolTable, NumberOfSymbols;
    uint16_t SizeOfOptionalHeader, Characteristics;
} COFF_HDR;
typedef struct {
    uint16_t Magic; uint8_t MajorLinker, MinorLinker;
    uint32_t SizeOfCode, SizeOfInit, SizeOfUninit, AddressOfEntryPoint, BaseOfCode;
    uint64_t ImageBase;
    uint32_t SectionAlignment, FileAlignment;
} OPT_HDR64_PREFIX;
typedef struct {
    char Name[8];
    uint32_t VirtualSize, VirtualAddress, SizeOfRawData, PointerToRawData;
    uint32_t PointerToRelocations, PointerToLinenumbers;
    uint16_t NumberOfRelocations, NumberOfLinenumbers;
    uint32_t Characteristics;
} SEC_HDR;
#pragma pack(pop)

int LdrParsePe(const uint8_t* Data, size_t Len, NIMBUS_PE_IMAGE* Out) {
    const DOS_HDR* dos;
    const COFF_HDR* coff;
    const OPT_HDR64_PREFIX* opt;
    uint32_t lfanew;
    if (!Data || !Out || Len < 64) return 0;
    memset(Out, 0, sizeof(*Out));
    dos = (const DOS_HDR*)Data;
    if (dos->e_magic != 0x5A4D) return 0; /* MZ */
    lfanew = (uint32_t)dos->e_lfanew;
    if (lfanew + sizeof(COFF_HDR) + 4 > Len) return 0;
    if (*(const uint32_t*)(Data + lfanew) != 0x00004550) return 0; /* PE\0\0 */
    coff = (const COFF_HDR*)(Data + lfanew);
    if (coff->Machine != 0x8664 && coff->Machine != 0x14c) return 0; /* AMD64 o i386 */
    if (coff->SizeOfOptionalHeader < 24) return 0;
    opt = (const OPT_HDR64_PREFIX*)(Data + lfanew + sizeof(COFF_HDR));
    /* Magic 0x20B = PE32+ ; 0x10B = PE32 */
    if (opt->Magic != 0x20B && opt->Magic != 0x10B) return 0;
    Out->Base = Data;
    Out->Size = Len;
    Out->EntryRva = opt->AddressOfEntryPoint;
    Out->NumSections = coff->NumberOfSections;
    Out->ImageBase = (uint32_t)(opt->ImageBase & 0xFFFFFFFFu);
    Out->Valid = 1;
    return 1;
}

uint32_t LdrRvaToOffset(const uint8_t* Data, size_t Len, uint32_t Rva) {
    const DOS_HDR* dos;
    const COFF_HDR* coff;
    const SEC_HDR* sec;
    uint32_t lfanew, i;
    if (!Data || Len < 64) return 0;
    dos = (const DOS_HDR*)Data;
    lfanew = (uint32_t)dos->e_lfanew;
    coff = (const COFF_HDR*)(Data + lfanew);
    sec = (const SEC_HDR*)(Data + lfanew + sizeof(COFF_HDR) + coff->SizeOfOptionalHeader);
    for (i = 0; i < coff->NumberOfSections; i++) {
        uint32_t va = sec[i].VirtualAddress;
        uint32_t vs = sec[i].VirtualSize ? sec[i].VirtualSize : sec[i].SizeOfRawData;
        if (Rva >= va && Rva < va + vs) {
            return sec[i].PointerToRawData + (Rva - va);
        }
    }
    return Rva; /* headers */
}
