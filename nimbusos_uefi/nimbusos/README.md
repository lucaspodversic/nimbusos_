# NimbusOS

**NimbusOS** is a clean-room, lightweight operating system with **Windows-like (Win11/12 style)** UX and **PE x64 app compatibility**, designed to keep the *system* footprint at **≤ 1 GB RAM** (idle desktop target ≈ 575 MB).

> **This is not Windows.** The product name is **NimbusOS**. There is **no Microsoft source code** here. The implementation is a *clean-room* reimplementation guided by publicly documented Win32/NT APIs, the PE/COFF format, and lessons from projects such as ReactOS and Wine.

---

## Español

### Objetivos
| Meta | Valor |
|------|-------|
| RAM en reposo (escritorio) | ≤ 450–575 MB |
| Techo del **sistema** | ≤ 1 GB |
| Disco instalado | ≤ 8 GB |
| Arranque en frío (NVMe) | ≤ 6 s |
| Compatibilidad | Binarios PE x64 sin recompilar |
| Hardware | x86-64 UEFI (TPM opcional) |

### Aviso clean-room
- No se copia código, binarios ni assets propietarios de Microsoft.
- Las APIs se reimplementan a partir de documentación pública.
- Los números de syscall son un ABI **propio** de NimbusOS (documentado), no una copia de la tabla de Windows.
- Marcas: no usamos el nombre “Windows” como marca del producto.

### Qué incluye este repositorio
- `boot/` · `hal/` · `kernel/` — núcleo host-testeable (`NIMBUS_HOST=1`)
- `drivers/` — NVMe/AHCI, Virtio-net, xHCI, TCP/IP stubs, **NimbusFS** (Rust)
- `userland/` — ntdll loader PE, Win32 (`kernel32`/`user32`/`gdi32`/`advapi32`/`ws2_32`), DX→Vulkan shims, win32k_server
- `services/` — SCM (bajo demanda), compositor, memory governor, update A/B, installer MSI/MSIX, defender-lite, …
- `shell/` — taskbar, start, explorer, settings (sin WebView)
- `compat/` · `docs/` · `tests/` · `build/`

### Cómo construir y testear (host Linux)
```bash
# C / kernel / Win32 / drivers
cmake -S . -B out -DNIMBUS_HOST=ON
cmake --build out -j
ctest --test-dir out --output-on-failure

# Rust: NimbusFS, SCM, shell, servicios
cargo test --workspace
```

### Alcance honesto
| Corre en host hoy | Requiere QEMU/hardware |
|---|---|
| Tests del kernel (scheduler, zram, registry, ACL, PE, budgets) | Boot BIOS (`out/nimbusos.iso`) y UEFI/OVMF (`out/nimbusos_uefi.img`) en QEMU |
| NimbusFS `cargo test` (format, journal, snapshots, ADS) | Montaje block-device |
| SCM on-demand, shell unit tests, compositor software | Presentación Vulkan/GPU |
| Parser PE + Reg* advapi32 + drivers virtio/nvme/xhci IRP | IRQ/DMA reales |
| DX shims (superficie de API) | Traducción DXVK completa + GPU |

### ISO arrancable (QEMU)

Ruta Multiboot2 + GRUB (BIOS/`i386-pc`). Si el toolchain UEFI completo no está, este fallback arranca en QEMU sin OVMF.

```bash
# Dependencias (Debian/Ubuntu)
sudo apt install nasm gcc binutils grub-pc-bin grub-common xorriso mtools qemu-system-x86

# Construir kernel freestanding + ISO
python3 build/make_iso.py
# → out/nimbusos.iso   y   out/kernel.elf

# Probar arranque (captura serial, exit 0 si ve "NimbusOS")
./tests/boot/test_qemu_boot.sh

# Manual:
qemu-system-x86_64 -cdrom out/nimbusos.iso -m 256 -serial stdio -display none
```

El kernel freestanding vive en `boot/freestanding/` (serial COM1, banner **NimbusOS**, Mm bump, shell serie). No usa `NIMBUS_HOST`.

| Artefacto | Ruta |
|-----------|------|
| ISO | `out/nimbusos.iso` |
| Kernel ELF | `out/kernel.elf` |
| Script ISO | `build/make_iso.py` |
| Test QEMU | `tests/boot/test_qemu_boot.sh` |

Ruta BIOS: Multiboot2/GRUB (probada con `tests/boot/test_qemu_boot.sh`).

### Arranque UEFI real (OVMF)

`BOOTX64.EFI` (gnu-efi, `boot/uefi/app/bootx64.c`) lee `\NIMBUS\KERNEL.ELF` de la ESP, valida el ELF64, reserva su rango físico (1 MiB) con `AllocatePages`, copia los segmentos, obtiene el mapa de memoria, llama a `ExitBootServices` y salta a `uefi_entry64` del kernel (localizado mediante la cabecera `NMBSUEFI`). El kernel instala su propia GDT y paginación identidad 0–4 GiB, resume el mapa de memoria UEFI y entra en el mismo shell serie. **Es el mismo `kernel.elf` que arranca GRUB.**

```bash
sudo apt install gnu-efi ovmf mtools dosfstools gdisk xorriso nasm gcc binutils qemu-system-x86

python3 build/make_uefi.py
# → out/BOOTX64.EFI
# → out/nimbusos_uefi.img   (disco GPT 64 MiB, ESP FAT32: /EFI/BOOT/BOOTX64.EFI + /NIMBUS/KERNEL.ELF)
# → out/nimbusos_uefi.iso   (ISO El Torito EFI)

./tests/boot/test_qemu_uefi.sh         # disco GPT, exit 0 si el shell responde a 'about'
./tests/boot/test_qemu_uefi.sh --iso   # ISO UEFI

# Manual:
cp /usr/share/OVMF/OVMF_VARS_4M.fd /tmp/vars.fd
qemu-system-x86_64 -machine q35 -m 256 \
  -drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_CODE_4M.fd \
  -drive if=pflash,format=raw,file=/tmp/vars.fd \
  -drive format=raw,file=out/nimbusos_uefi.img \
  -net none -serial stdio -display none

# Todo (BIOS + UEFI) y pruebas:
./build/rebuild_all.sh
```

| Artefacto UEFI | Ruta |
|----------------|------|
| Cargador UEFI | `out/BOOTX64.EFI` |
| Disco GPT/ESP | `out/nimbusos_uefi.img` |
| ISO UEFI | `out/nimbusos_uefi.iso` |
| Script | `build/make_uefi.py` |
| Test | `tests/boot/test_qemu_uefi.sh` |

### Licencia
MIT (ver `LICENSE`). Contribuciones: `CONTRIBUTING.md` / `docs/contributing.md`.

---

## English

A clean-room OS that feels familiar to Windows 11/12 users, runs PE x64 apps, and keeps **OS** RAM ≤ 1 GB via on-demand services, user-mode window server, zram compression, page dedupe, and hard per-component budgets.

### Build (host)
```bash
cmake -S . -B out -DNIMBUS_HOST=ON && cmake --build out -j && ctest --test-dir out --output-on-failure
cargo test --workspace
```

### Bootable ISO (QEMU)
```bash
sudo apt install nasm gcc binutils grub-pc-bin grub-common xorriso mtools qemu-system-x86
python3 build/make_iso.py          # → out/nimbusos.iso
./tests/boot/test_qemu_boot.sh     # serial must show NimbusOS
```
Freestanding Multiboot2 kernel: `boot/freestanding/`. Toolchain fallback: GRUB BIOS (no OVMF required).

### UEFI boot (OVMF)
```bash
sudo apt install gnu-efi ovmf mtools dosfstools gdisk xorriso nasm gcc binutils qemu-system-x86
python3 build/make_uefi.py              # → out/BOOTX64.EFI, out/nimbusos_uefi.img (GPT+FAT32 ESP), out/nimbusos_uefi.iso
./tests/boot/test_qemu_uefi.sh          # OVMF via -drive if=pflash; checks the serial shell answers 'about'
./tests/boot/test_qemu_uefi.sh --iso
```
`BOOTX64.EFI` (gnu-efi) loads `\NIMBUS\KERNEL.ELF`, exits boot services and jumps to the kernel's 64-bit UEFI entry. The same `kernel.elf` boots from GRUB (Multiboot2) and from UEFI.

### Layout
- `boot/` — UEFI bootmgr, A/B, TPM measure
- `hal/` — APIC, ACPI, PCI, SMP
- `kernel/` — NT-compatible hybrid microkernel
- `drivers/` — storage, net, USB, **NimbusFS**
- `userland/` — ntdll, Win32, DirectX→Vulkan, window server
- `services/` — SCM, compositor, updates, installer, security
- `shell/` — taskbar / start / explorer

Product name: **NimbusOS** only.
