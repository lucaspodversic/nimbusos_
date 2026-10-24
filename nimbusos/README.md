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
| Tests del kernel (scheduler, zram, registry, ACL, PE, budgets) | Boot UEFI real |
| NimbusFS `cargo test` (format, journal, snapshots, ADS) | Montaje block-device |
| SCM on-demand, shell unit tests, compositor software | Presentación Vulkan/GPU |
| Parser PE + Reg* advapi32 + drivers virtio/nvme/xhci IRP | IRQ/DMA reales |
| DX shims (superficie de API) | Traducción DXVK completa + GPU |

Esto es una **arquitectura ejecutable y testeable**, no un ISO listo para instalar en PC todavía. El siguiente paso natural es imagen UEFI + QEMU.

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

### Layout
- `boot/` — UEFI bootmgr, A/B, TPM measure
- `hal/` — APIC, ACPI, PCI, SMP
- `kernel/` — NT-compatible hybrid microkernel
- `drivers/` — storage, net, USB, **NimbusFS**
- `userland/` — ntdll, Win32, DirectX→Vulkan, window server
- `services/` — SCM, compositor, updates, installer, security
- `shell/` — taskbar / start / explorer

Product name: **NimbusOS** only.
