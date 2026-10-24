; =============================================================================
; NimbusOS — arranque Multiboot2 → modo largo x86_64
;
; Flujo:
;   1. GRUB encuentra el encabezado Multiboot2 y carga el ELF.
;   2. Salta a _start en modo protegido 32-bit (CR0.PE=1, paging off).
;   3. Configuramos tablas de páginas identidad 0–4 GiB (2 MiB pages).
;   4. Activamos PAE + LME + paging → modo largo.
;   5. Saltamos a kmain64 (C freestanding).
;
; Comentarios en español (requisito del proyecto). Marca: NimbusOS.
; =============================================================================

BITS 32

; --- Constantes Multiboot2 ---------------------------------------------------
MB2_MAGIC           equ 0xE85250D6
MB2_ARCH_I386       equ 0
MB2_HEADER_LEN      equ multiboot_header_end - multiboot_header
MB2_CHECKSUM        equ -(MB2_MAGIC + MB2_ARCH_I386 + MB2_HEADER_LEN)

; Tag types
MB2_TAG_END         equ 0
MB2_TAG_INFO_REQ    equ 1
MB2_TAG_ADDR        equ 2
MB2_TAG_ENTRY       equ 3
MB2_TAG_FLAGS       equ 4
MB2_TAG_FB          equ 5
MB2_TAG_MODULE_ALIGN equ 6

section .multiboot2
align 8
multiboot_header:
    dd MB2_MAGIC
    dd MB2_ARCH_I386
    dd MB2_HEADER_LEN
    dd MB2_CHECKSUM

    ; Tag: console flags — preferimos consola serial / texto
    align 8
    dw 4                ; type = flags
    dw 0                ; flags
    dd 12               ; size
    dd 3                ; console_flags: EGA text + console required bits optional
    ; pad to 8
    dd 0

    ; Tag fin
    align 8
    dw MB2_TAG_END
    dw 0
    dd 8
multiboot_header_end:

; --- Cabecera de traspaso UEFI propia de NimbusOS -----------------------------
; BOOTX64.EFI busca la firma 'NMBSUEFI' en los primeros 32 KiB de la imagen
; cargada y salta a la dirección absoluta (64-bit) que sigue. Así un único
; kernel.elf sirve para ambas rutas: GRUB entra por _start (32-bit, e_entry) y
; UEFI entra por uefi_entry64 (ya en modo largo, tras ExitBootServices).
align 8
global nimbus_uefi_handoff
nimbus_uefi_handoff:
    db 'NMBSUEFI'               ; firma (8 bytes)
    dq uefi_entry64             ; punto de entrada 64-bit
    dd 1                        ; versión del protocolo de traspaso
    dd 0                        ; reservado

; --- Código de arranque 32-bit -----------------------------------------------
section .text.boot
global _start
extern kmain64
extern __stack_top
extern __bss_start
extern __bss_end

_start:
    cli
    ; Guardamos el puntero Multiboot (ebx) y el magic (eax) por si kmain los usa
    mov [mb_magic], eax
    mov [mb_info],  ebx

    ; Limpiar BSS (desde __bss_start hasta __bss_end)
    mov edi, __bss_start
    mov ecx, __bss_end
    sub ecx, edi
    xor eax, eax
    rep stosb

    ; Restaurar magic/info tras limpiar BSS (estaban en .bss; los reescribimos)
    ; En realidad mb_magic/mb_info están en .data — no los borramos. OK.

    ; --- Page tables: identidad 0–4 GiB con páginas de 2 MiB -----------------
    ; PML4[0] -> PDPT
    ; PDPT[0..3] -> PD0..PD3
    ; Cada PD tiene 512 entradas de 2 MiB → 1 GiB por PD → 4 GiB total
    mov edi, pml4
    mov cr3, edi

    ; PML4[0] = pdpt | PRESENT | RW
    mov eax, pdpt
    or  eax, 0x03
    mov [pml4], eax
    mov dword [pml4 + 4], 0

    ; PDPT[0]=pd0, [1]=pd1, [2]=pd2, [3]=pd3
    mov eax, pd0
    or  eax, 0x03
    mov [pdpt + 0*8], eax
    mov dword [pdpt + 0*8 + 4], 0
    mov eax, pd1
    or  eax, 0x03
    mov [pdpt + 1*8], eax
    mov dword [pdpt + 1*8 + 4], 0
    mov eax, pd2
    or  eax, 0x03
    mov [pdpt + 2*8], eax
    mov dword [pdpt + 2*8 + 4], 0
    mov eax, pd3
    or  eax, 0x03
    mov [pdpt + 3*8], eax
    mov dword [pdpt + 3*8 + 4], 0

    ; Rellenar las 4 tablas PD con entradas 2 MiB (bit PS=1)
    mov edi, pd0
    xor ebx, ebx            ; dirección física base
    mov ecx, 512 * 4        ; 2048 entradas
.fill_pd:
    mov eax, ebx
    or  eax, 0x83           ; PRESENT | RW | PS(2MiB)
    stosd
    xor eax, eax
    stosd
    add ebx, 0x200000       ; +2 MiB
    loop .fill_pd

    ; CR4.PAE = 1
    mov eax, cr4
    or  eax, (1 << 5)
    mov cr4, eax

    ; EFER.LME = 1 (MSR 0xC0000080)
    mov ecx, 0xC0000080
    rdmsr
    or  eax, (1 << 8)
    wrmsr

    ; CR0.PG = 1 (activar paginación → compatibility mode)
    mov eax, cr0
    or  eax, (1 << 31)
    mov cr0, eax

    ; Cargar GDT 64-bit y far-jump a código 64
    lgdt [gdt64_ptr]
    jmp 0x08:long_mode_entry

; ---------------------------------------------------------------------------
BITS 64
long_mode_entry:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax

    mov rsp, __stack_top
    and rsp, ~0xF

    ; Pasar magic e info a kmain64(uint32_t magic, uint64_t info)
    mov edi, dword [mb_magic]
    mov esi, dword [mb_info]
    xor rdx, rdx
    call kmain64

.hang:
    hlt
    jmp .hang

;
; ---------------------------------------------------------------------------
; uefi_entry64 — entrada desde BOOTX64.EFI (System V ABI):
;   edi = magic NIMBUS_UEFI_MAGIC (0x4E554546, "NUEF")
;   rsi = puntero físico a NIMBUS_UEFI_BOOTINFO (mapa de memoria UEFI)
; La CPU ya está en modo largo con las tablas de páginas del firmware
; (identidad). Aun así tomamos el control completo de la máquina:
;   1. cli + nuestra GDT (CS=0x08, DS=0x10) — la GDT de OVMF ya no es nuestra.
;   2. Tablas de páginas propias: identidad 0–4 GiB con páginas de 2 MiB
;      (las mismas pml4/pdpt/pd0..pd3 que usa la ruta Multiboot2).
;   3. Pila propia (__stack_top) y salto a kmain64(magic, info).
; ---------------------------------------------------------------------------
global uefi_entry64
uefi_entry64:
    cli
    mov r12d, edi               ; preservar magic
    mov r13, rsi                ; preservar puntero bootinfo
    mov rsp, __stack_top        ; pila del kernel (en .bss, ya a cero)
    and rsp, ~0xF

    ; 1) GDT propia y recarga de CS mediante retfq (no existe "jmp far" directo)
    lgdt [gdt64_ptr]
    push qword 0x08
    lea rax, [rel .reload_cs]
    push rax
    retfq
.reload_cs:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax

    ; 2) Tablas de páginas identidad 0–4 GiB (2048 entradas de 2 MiB)
    mov rax, pdpt
    or  rax, 0x03
    mov [pml4], rax
    mov rax, pd0
    or  rax, 0x03
    mov [pdpt + 0*8], rax
    mov rax, pd1
    or  rax, 0x03
    mov [pdpt + 1*8], rax
    mov rax, pd2
    or  rax, 0x03
    mov [pdpt + 2*8], rax
    mov rax, pd3
    or  rax, 0x03
    mov [pdpt + 3*8], rax

    mov rdi, pd0
    xor rbx, rbx                ; dirección física base
    mov rcx, 512 * 4
.fill_pd64:
    mov rax, rbx
    or  rax, 0x83               ; PRESENT | RW | PS (2 MiB)
    mov [rdi], rax
    add rdi, 8
    add rbx, 0x200000
    dec rcx
    jnz .fill_pd64

    mov rax, pml4
    mov cr3, rax                ; activar nuestras tablas (vacía la TLB)

    ; 3) kmain64(magic, info)
    mov edi, r12d
    mov rsi, r13
    xor edx, edx
    call kmain64
.hang64:
    hlt
    jmp .hang64

; --- Datos -------------------------------------------------------------------
section .data
align 4
mb_magic: dd 0
mb_info:  dd 0

align 16
gdt64:
    dq 0                                    ; null
    dq 0x00AF9A000000FFFF                   ; code 64: L=1, D=0, P=1, S=1, exec/read
    dq 0x00CF92000000FFFF                   ; data 64: P=1, S=1, write
gdt64_end:

gdt64_ptr:
    dw gdt64_end - gdt64 - 1
    dq gdt64

; Tablas de páginas (deben estar alineadas a 4 KiB; van en .bss)
section .bss
align 4096
pml4: resb 4096
pdpt: resb 4096
pd0:  resb 4096
pd1:  resb 4096
pd2:  resb 4096
pd3:  resb 4096
