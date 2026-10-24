/* NimbusOS — parser/loader PE/COFF x64 (clean-room).
 *
 * Implementa el subconjunto necesario para:
 *  - Validar DOS stub + PE signature
 *  - Leer COFF File Header + Optional Header PE32+
 *  - Enumerar secciones
 *  - Mapear imagen (copia de headers + secciones raw→VA)
 *  - Fabricar un PE mínimo x64 para tests (PeCraftMinimalX64)
 *
 * NO copia código de Microsoft; se basa en la especificación pública PE/COFF.
 */
#include "ps.h"
#include "../host/host_compat.h"

#pragma pack(push, 1)
typedef struct {
    USHORT e_magic;      /* 'MZ' */
    USHORT e_cblp, e_cp, e_crlc, e_cparhdr, e_minalloc, e_maxalloc;
    USHORT e_ss, e_sp, e_csum, e_ip, e_cs, e_lfarlc, e_ovno;
    USHORT e_res[4];
    USHORT e_oemid, e_oeminfo;
    USHORT e_res2[10];
    LONG   e_lfanew;
} IMAGE_DOS_HEADER;

typedef struct {
    ULONG  Signature;    /* 'PE\0\0' */
    USHORT Machine;
    USHORT NumberOfSections;
    ULONG  TimeDateStamp;
    ULONG  PointerToSymbolTable;
    ULONG  NumberOfSymbols;
    USHORT SizeOfOptionalHeader;
    USHORT Characteristics;
} IMAGE_FILE_HEADER;

typedef struct {
    USHORT Magic;        /* 0x20B = PE32+ */
    UCHAR  MajorLinkerVersion, MinorLinkerVersion;
    ULONG  SizeOfCode, SizeOfInitializedData, SizeOfUninitializedData;
    ULONG  AddressOfEntryPoint;
    ULONG  BaseOfCode;
    ULONGLONG ImageBase;
    ULONG  SectionAlignment, FileAlignment;
    USHORT MajorOperatingSystemVersion, MinorOperatingSystemVersion;
    USHORT MajorImageVersion, MinorImageVersion;
    USHORT MajorSubsystemVersion, MinorSubsystemVersion;
    ULONG  Win32VersionValue;
    ULONG  SizeOfImage;
    ULONG  SizeOfHeaders;
    ULONG  CheckSum;
    USHORT Subsystem;
    USHORT DllCharacteristics;
    ULONGLONG SizeOfStackReserve, SizeOfStackCommit;
    ULONGLONG SizeOfHeapReserve, SizeOfHeapCommit;
    ULONG  LoaderFlags;
    ULONG  NumberOfRvaAndSizes;
} IMAGE_OPTIONAL_HEADER64;

typedef struct {
    CHAR  Name[8];
    ULONG VirtualSize;
    ULONG VirtualAddress;
    ULONG SizeOfRawData;
    ULONG PointerToRawData;
    ULONG PointerToRelocations;
    ULONG PointerToLinenumbers;
    USHORT NumberOfRelocations;
    USHORT NumberOfLinenumbers;
    ULONG Characteristics;
} IMAGE_SECTION_HEADER;
#pragma pack(pop)

#define IMAGE_DOS_SIGNATURE 0x5A4D
#define IMAGE_NT_SIGNATURE  0x00004550
#define IMAGE_FILE_MACHINE_AMD64 0x8664
#define IMAGE_NT_OPTIONAL_HDR64_MAGIC 0x20B

NTSTATUS PeParseImage(const void* FileData, SIZE_T FileSize, PPE_IMAGE_INFO Info) {
    const UCHAR* base = (const UCHAR*)FileData;
    const IMAGE_DOS_HEADER* dos;
    const IMAGE_FILE_HEADER* fh;
    const IMAGE_OPTIONAL_HEADER64* opt;
    ULONG pe_off;
    if (!FileData || !Info || FileSize < sizeof(IMAGE_DOS_HEADER))
        return STATUS_INVALID_IMAGE_FORMAT;
    nimbus_host_memset(Info, 0, sizeof(*Info));
    dos = (const IMAGE_DOS_HEADER*)base;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) return STATUS_INVALID_IMAGE_FORMAT;
    pe_off = (ULONG)dos->e_lfanew;
    if (pe_off + 4 + sizeof(IMAGE_FILE_HEADER) + sizeof(IMAGE_OPTIONAL_HEADER64) > FileSize)
        return STATUS_INVALID_IMAGE_FORMAT;
    if (*(const ULONG*)(base + pe_off) != IMAGE_NT_SIGNATURE)
        return STATUS_INVALID_IMAGE_FORMAT;
    fh = (const IMAGE_FILE_HEADER*)(base + pe_off + 4);
    if (fh->Machine != IMAGE_FILE_MACHINE_AMD64) return STATUS_INVALID_IMAGE_FORMAT;
    opt = (const IMAGE_OPTIONAL_HEADER64*)(base + pe_off + 4 + sizeof(IMAGE_FILE_HEADER));
    if (opt->Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC) return STATUS_INVALID_IMAGE_FORMAT;

    Info->Machine = fh->Machine;
    Info->NumberOfSections = fh->NumberOfSections;
    Info->AddressOfEntryPoint = opt->AddressOfEntryPoint;
    Info->ImageBase = (ULONG)(opt->ImageBase & 0xFFFFFFFFULL);
    Info->SizeOfImage = opt->SizeOfImage;
    Info->SizeOfHeaders = opt->SizeOfHeaders;
    Info->Subsystem = opt->Subsystem;
    Info->FileData = base;
    Info->FileSize = FileSize;
    return STATUS_SUCCESS;
}

NTSTATUS PeMapImage(PPE_IMAGE_INFO Info, PVOID* OutBase) {
    UCHAR* mapped;
    const UCHAR* file;
    ULONG pe_off, i;
    const IMAGE_DOS_HEADER* dos;
    const IMAGE_FILE_HEADER* fh;
    const IMAGE_SECTION_HEADER* sec;
    if (!Info || !OutBase || !Info->FileData) return STATUS_INVALID_PARAMETER;
    mapped = (UCHAR*)nimbus_host_calloc(1, Info->SizeOfImage ? Info->SizeOfImage : PAGE_SIZE);
    if (!mapped) return STATUS_NO_MEMORY;
    file = Info->FileData;
    /* Copiar headers */
    {
        SIZE_T hdr = Info->SizeOfHeaders;
        if (hdr > Info->FileSize) hdr = Info->FileSize;
        if (hdr > Info->SizeOfImage) hdr = Info->SizeOfImage;
        nimbus_host_memcpy(mapped, file, hdr);
    }
    dos = (const IMAGE_DOS_HEADER*)file;
    pe_off = (ULONG)dos->e_lfanew;
    fh = (const IMAGE_FILE_HEADER*)(file + pe_off + 4);
    sec = (const IMAGE_SECTION_HEADER*)(
        file + pe_off + 4 + sizeof(IMAGE_FILE_HEADER) + fh->SizeOfOptionalHeader);
    for (i = 0; i < fh->NumberOfSections; i++) {
        ULONG va = sec[i].VirtualAddress;
        ULONG raw = sec[i].PointerToRawData;
        ULONG rsz = sec[i].SizeOfRawData;
        ULONG vsz = sec[i].VirtualSize ? sec[i].VirtualSize : rsz;
        if (raw == 0 || rsz == 0) continue;
        if ((SIZE_T)raw + rsz > Info->FileSize) continue;
        if ((SIZE_T)va + vsz > Info->SizeOfImage) continue;
        nimbus_host_memcpy(mapped + va, file + raw, rsz < vsz ? rsz : vsz);
    }
    Info->MappedBase = mapped;
    *OutBase = mapped;
    return STATUS_SUCCESS;
}

NTSTATUS PsLoadPeImage(PEPROCESS Process, const void* FileData, SIZE_T FileSize, PVOID* OutBase) {
    PE_IMAGE_INFO info;
    NTSTATUS st;
    PVOID base = NULL;
    st = PeParseImage(FileData, FileSize, &info);
    if (st != STATUS_SUCCESS) return st;
    st = PeMapImage(&info, &base);
    if (st != STATUS_SUCCESS) return st;
    if (Process) {
        Process->SectionBase = base;
        Process->ImageSize = info.SizeOfImage;
    }
    if (OutBase) *OutBase = base;
    return STATUS_SUCCESS;
}

/* Fabrica un PE64 mínimo válido en OutBuf. Devuelve tamaño usado. */
SIZE_T PeCraftMinimalX64(void* OutBuf, SIZE_T Cap, ULONG EntryRva) {
    UCHAR* b = (UCHAR*)OutBuf;
    IMAGE_DOS_HEADER* dos;
    IMAGE_FILE_HEADER* fh;
    IMAGE_OPTIONAL_HEADER64* opt;
    IMAGE_SECTION_HEADER* sec;
    ULONG pe_off = 0x80;
    ULONG headers = 0x200;
    ULONG sec_va = 0x1000;
    ULONG sec_raw = 0x200;
    ULONG sec_size = 0x200;
    ULONG image_size = 0x2000;
    SIZE_T total = sec_raw + sec_size;
    UCHAR* code;

    if (!OutBuf || Cap < total) return 0;
    nimbus_host_memset(b, 0, total);

    dos = (IMAGE_DOS_HEADER*)b;
    dos->e_magic = IMAGE_DOS_SIGNATURE;
    dos->e_lfanew = (LONG)pe_off;

    *(ULONG*)(b + pe_off) = IMAGE_NT_SIGNATURE;
    fh = (IMAGE_FILE_HEADER*)(b + pe_off + 4);
    fh->Machine = IMAGE_FILE_MACHINE_AMD64;
    fh->NumberOfSections = 1;
    fh->SizeOfOptionalHeader = sizeof(IMAGE_OPTIONAL_HEADER64);
    fh->Characteristics = 0x22; /* EXECUTABLE_IMAGE | LARGE_ADDRESS_AWARE */

    opt = (IMAGE_OPTIONAL_HEADER64*)(b + pe_off + 4 + sizeof(IMAGE_FILE_HEADER));
    opt->Magic = IMAGE_NT_OPTIONAL_HDR64_MAGIC;
    opt->AddressOfEntryPoint = EntryRva ? EntryRva : sec_va;
    opt->ImageBase = 0x140000000ULL;
    opt->SectionAlignment = 0x1000;
    opt->FileAlignment = 0x200;
    opt->MajorSubsystemVersion = 6;
    opt->SizeOfImage = image_size;
    opt->SizeOfHeaders = headers;
    opt->Subsystem = 3; /* CONSOLE */
    opt->SizeOfStackReserve = 0x100000;
    opt->SizeOfHeapReserve = 0x100000;
    opt->NumberOfRvaAndSizes = 16;
    opt->BaseOfCode = sec_va;
    opt->SizeOfCode = sec_size;

    sec = (IMAGE_SECTION_HEADER*)((UCHAR*)opt + sizeof(IMAGE_OPTIONAL_HEADER64));
    nimbus_host_memcpy(sec->Name, ".text\0\0\0", 8);
    sec->VirtualSize = sec_size;
    sec->VirtualAddress = sec_va;
    sec->SizeOfRawData = sec_size;
    sec->PointerToRawData = sec_raw;
    sec->Characteristics = 0x60000020; /* CODE | EXECUTE | READ */

    /* Código: bytes "ret" (0xC3) en el entry */
    code = b + sec_raw;
    code[0] = 0xC3;
    return total;
}
