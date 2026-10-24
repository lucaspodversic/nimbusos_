/* =============================================================================
 * NimbusOS — BOOTX64.EFI: cargador UEFI real (gnu-efi, x86_64).
 *
 * Flujo completo:
 *   1. efi_main recibe ImageHandle + SystemTable del firmware (OVMF/PC real).
 *   2. Abre el volumen desde el que arrancó (la ESP FAT32) mediante
 *      EFI_LOADED_IMAGE → DeviceHandle → EFI_SIMPLE_FILE_SYSTEM.
 *   3. Lee \NIMBUS\KERNEL.ELF entero a memoria (pool).
 *   4. Valida el ELF64 (x86_64, ET_EXEC) y reserva con AllocatePages
 *      (AllocateAddress) el rango físico exacto de sus PT_LOAD (enlazado en
 *      1 MiB; el kernel NO es reubicable).
 *   5. Copia cada segmento y pone a cero p_memsz - p_filesz (.bss + pila).
 *   6. Busca la cabecera de traspaso 'NMBSUEFI' para obtener uefi_entry64.
 *   7. Prepara NIMBUS_UEFI_BOOTINFO, obtiene el mapa de memoria final y llama
 *      a ExitBootServices (reintento si el MapKey cambió).
 *   8. Ya sin servicios de arranque: escribe por COM1 directamente y salta al
 *      kernel en modo largo (System V ABI), que instala GDT/paginación propias.
 *
 * Clean-room: escrito desde la especificación UEFI 2.x pública y la ABI ELF.
 * Producto: NimbusOS.
 * ===========================================================================*/
#include <efi.h>
#include <efilib.h>
#include "../../freestanding/nimbus_bootinfo.h"

#define KERNEL_PATH L"\\NIMBUS\\KERNEL.ELF"

/* --- Mínimo ELF64 (solo lo que necesitamos) -------------------------------- */
typedef struct {
    UINT8  e_ident[16];
    UINT16 e_type, e_machine;
    UINT32 e_version;
    UINT64 e_entry, e_phoff, e_shoff;
    UINT32 e_flags;
    UINT16 e_ehsize, e_phentsize, e_phnum, e_shentsize, e_shnum, e_shstrndx;
} Elf64_Ehdr;

typedef struct {
    UINT32 p_type, p_flags;
    UINT64 p_offset, p_vaddr, p_paddr, p_filesz, p_memsz, p_align;
} Elf64_Phdr;

#define PT_LOAD   1
#define EM_X86_64 62
#define ET_EXEC   2

/* --- COM1 directo (válido también tras ExitBootServices) ------------------- */
static inline void io_outb(UINT16 port, UINT8 v) { __asm__ volatile("outb %0,%1"::"a"(v),"Nd"(port)); }
static inline UINT8 io_inb(UINT16 port) { UINT8 r; __asm__ volatile("inb %1,%0":"=a"(r):"Nd"(port)); return r; }
static void com1_puts(const char *s) {
    for (; *s; s++) {
        if (*s == '\n') { while (!(io_inb(0x3FD) & 0x20)); io_outb(0x3F8, '\r'); }
        while (!(io_inb(0x3FD) & 0x20));
        io_outb(0x3F8, (UINT8)*s);
    }
}

/* Copia/relleno byte a byte propios: no dependen de la convención de llamada
 * (EFIAPI/ms_abi vs sysv) con la que se compiló libefi en cada distro. */
static void nb_memcpy(UINT8 *d, const UINT8 *s, UINT64 n) { while (n--) *d++ = *s++; }
static void nb_memset(UINT8 *d, UINT8 v, UINT64 n)       { while (n--) *d++ = v; }

/* Entrada del kernel: System V ABI explícito (gnu-efi puede compilar con MS ABI). */
typedef void (__attribute__((sysv_abi)) *KERNEL_ENTRY64)(UINT32 magic, UINT64 info);

static EFI_STATUS fail(CHAR16 *msg, EFI_STATUS st) {
    Print(L"NimbusOS bootx64: ERROR %s (%r)\n", msg, st);
    return st;
}

/* Lee un fichero completo del volumen de arranque a un buffer del pool. */
static EFI_STATUS read_kernel(EFI_HANDLE image, VOID **buf, UINTN *size) {
    EFI_STATUS st;
    EFI_LOADED_IMAGE *li = NULL;
    st = uefi_call_wrapper(BS->HandleProtocol, 3, image, &LoadedImageProtocol, (VOID **)&li);
    if (EFI_ERROR(st)) return fail(L"LoadedImage", st);

    EFI_FILE_HANDLE root = LibOpenRoot(li->DeviceHandle);
    if (!root) return fail(L"OpenRoot (ESP)", EFI_NOT_FOUND);

    EFI_FILE_HANDLE f;
    st = uefi_call_wrapper(root->Open, 5, root, &f, KERNEL_PATH, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(st)) return fail(L"abrir \\NIMBUS\\KERNEL.ELF", st);

    EFI_FILE_INFO *info = LibFileInfo(f);
    if (!info) return fail(L"FileInfo", EFI_DEVICE_ERROR);
    *size = (UINTN)info->FileSize;
    FreePool(info);

    *buf = AllocatePool(*size);
    if (!*buf) return fail(L"AllocatePool kernel", EFI_OUT_OF_RESOURCES);
    UINTN rd = *size;
    st = uefi_call_wrapper(f->Read, 3, f, &rd, *buf);
    uefi_call_wrapper(f->Close, 1, f);
    if (EFI_ERROR(st) || rd != *size) return fail(L"Read kernel", st);
    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI efi_main(EFI_HANDLE image, EFI_SYSTEM_TABLE *systab) {
    EFI_STATUS st;
    InitializeLib(image, systab);

    Print(L"\nNimbusOS UEFI bootmgr (BOOTX64.EFI) - clean-room\n");
    Print(L"Firmware: %s rev %d.%d\n", systab->FirmwareVendor,
          systab->FirmwareRevision >> 16, systab->FirmwareRevision & 0xFFFF);

    /* 1) Cargar el ELF del kernel desde la ESP */
    VOID *file = NULL; UINTN file_size = 0;
    st = read_kernel(image, &file, &file_size);
    if (EFI_ERROR(st)) return st;
    Print(L"Kernel: %s (%d bytes)\n", KERNEL_PATH, file_size);

    /* 2) Validar ELF64 x86_64 */
    Elf64_Ehdr *eh = (Elf64_Ehdr *)file;
    if (file_size < sizeof(*eh) || eh->e_ident[0] != 0x7F || eh->e_ident[1] != 'E' ||
        eh->e_ident[2] != 'L' || eh->e_ident[3] != 'F' || eh->e_ident[4] != 2 ||
        eh->e_machine != EM_X86_64 || eh->e_type != ET_EXEC)
        return fail(L"kernel no es ELF64 x86_64 ET_EXEC", EFI_LOAD_ERROR);

    /* 3) Rango físico total de los PT_LOAD */
    UINT64 lo = ~0ULL, hi = 0;
    for (UINT16 i = 0; i < eh->e_phnum; i++) {
        Elf64_Phdr *ph = (Elf64_Phdr *)((UINT8 *)file + eh->e_phoff + (UINT64)i * eh->e_phentsize);
        if (ph->p_type != PT_LOAD || ph->p_memsz == 0) continue;
        if (ph->p_paddr < lo) lo = ph->p_paddr;
        if (ph->p_paddr + ph->p_memsz > hi) hi = ph->p_paddr + ph->p_memsz;
    }
    if (hi <= lo) return fail(L"sin segmentos PT_LOAD", EFI_LOAD_ERROR);
    lo &= ~0xFFFULL;
    UINTN pages = (UINTN)((hi - lo + 0xFFF) >> 12);

    /* 4) Reservar exactamente [lo, hi) — el kernel está enlazado en 1 MiB */
    EFI_PHYSICAL_ADDRESS base = lo;
    st = uefi_call_wrapper(BS->AllocatePages, 4, AllocateAddress, EfiLoaderData, pages, &base);
    if (EFI_ERROR(st)) return fail(L"AllocatePages(AllocateAddress) rango del kernel", st);
    Print(L"Reservado: 0x%lx - 0x%lx (%d paginas)\n", lo, hi, pages);

    /* 5) Copiar segmentos y poner a cero .bss/pila */
    for (UINT16 i = 0; i < eh->e_phnum; i++) {
        Elf64_Phdr *ph = (Elf64_Phdr *)((UINT8 *)file + eh->e_phoff + (UINT64)i * eh->e_phentsize);
        if (ph->p_type != PT_LOAD || ph->p_memsz == 0) continue;
        if (ph->p_offset + ph->p_filesz > file_size) return fail(L"segmento fuera del fichero", EFI_LOAD_ERROR);
        nb_memcpy((UINT8 *)(UINTN)ph->p_paddr, (UINT8 *)file + ph->p_offset, ph->p_filesz);
        nb_memset((UINT8 *)(UINTN)(ph->p_paddr + ph->p_filesz), 0, ph->p_memsz - ph->p_filesz);
    }

    /* 6) Localizar la cabecera de traspaso 'NMBSUEFI' (primeros 32 KiB) */
    UINT64 entry = 0;
    UINT8 *img = (UINT8 *)(UINTN)lo;
    UINTN scan = (UINTN)((hi - lo) < 0x8000 ? (hi - lo) : 0x8000);
    static const UINT8 sig[8] = { 'N','M','B','S','U','E','F','I' };
    for (UINTN off = 0; off + 16 <= scan && !entry; off += 8) {
        UINTN k = 0;
        while (k < 8 && img[off + k] == sig[k]) k++;      /* memcmp propio, sin libefi */
        if (k == 8) entry = *(volatile UINT64 *)(img + off + 8);
    }
    if (!entry) return fail(L"cabecera NMBSUEFI no encontrada", EFI_LOAD_ERROR);
    Print(L"Entrada UEFI del kernel: 0x%lx\n", entry);

    /* 7) Bootinfo + RSDP ACPI (opcional) */
    NIMBUS_UEFI_BOOTINFO *bi = AllocatePool(sizeof(*bi));
    if (!bi) return fail(L"AllocatePool bootinfo", EFI_OUT_OF_RESOURCES);
    nb_memset((UINT8 *)bi, 0, sizeof(*bi));
    bi->Magic = NIMBUS_UEFI_MAGIC;
    bi->Version = NIMBUS_BOOTINFO_VERSION;
    bi->KernelBase = lo;
    bi->KernelSize = (UINT64)pages << 12;
    for (UINTN i = 0; i < systab->NumberOfTableEntries; i++) {
        if (CompareGuid(&systab->ConfigurationTable[i].VendorGuid, &AcpiTableGuid) == 0)
            bi->AcpiRsdp = (UINT64)(UINTN)systab->ConfigurationTable[i].VendorTable;
    }

    /* Mapa de memoria: reservamos holgura (+8 descriptores) porque
     * AllocatePool del propio buffer puede partir una región. */
    UINTN map_size = 0, map_key = 0, desc_size = 0; UINT32 desc_ver = 0;
    EFI_MEMORY_DESCRIPTOR *map = NULL;
    st = uefi_call_wrapper(BS->GetMemoryMap, 5, &map_size, map, &map_key, &desc_size, &desc_ver);
    map_size += 8 * desc_size;
    map = AllocatePool(map_size);
    if (!map) return fail(L"AllocatePool memmap", EFI_OUT_OF_RESOURCES);
    UINTN cap = map_size;

    Print(L"ExitBootServices: entregando la maquina a NimbusOS...\n");

    for (int attempt = 0; attempt < 4; attempt++) {
        map_size = cap;
        st = uefi_call_wrapper(BS->GetMemoryMap, 5, &map_size, map, &map_key, &desc_size, &desc_ver);
        if (EFI_ERROR(st)) return fail(L"GetMemoryMap", st);
        /* ¡Nada de Print/AllocatePool entre GetMemoryMap y ExitBootServices! */
        st = uefi_call_wrapper(BS->ExitBootServices, 2, image, map_key);
        if (!EFI_ERROR(st)) break;
    }
    if (EFI_ERROR(st)) return fail(L"ExitBootServices", st);

    /* 8) Sin servicios de arranque: solo E/S directa */
    bi->MemMap = (UINT64)(UINTN)map;
    bi->MemMapSize = map_size;
    bi->MemMapDescSize = desc_size;
    com1_puts("NimbusOS bootx64: ExitBootServices OK -> saltando al kernel (long mode)\n");

    __asm__ volatile("cli");
    ((KERNEL_ENTRY64)(UINTN)entry)(NIMBUS_UEFI_MAGIC, (UINT64)(UINTN)bi);
    /* El kernel no retorna; si lo hiciera, detenemos la CPU. */
    for (;;) __asm__ volatile("hlt");
    return EFI_SUCCESS;
}
