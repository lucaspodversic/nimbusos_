# NimbusOS — Arquitectura de un Windows 11/12 liviano (≤ 1 GB de RAM) compatible con apps x64

> Documento de diseño. Es una arquitectura *clean-room* (sin código de Microsoft): el código fuente de Windows es propietario, así que todo se reimplementa a partir de la documentación pública de la API Win32/NT, los formatos PE/COFF, y la experiencia de proyectos como ReactOS y Wine.

---

## 1. Objetivos y restricciones

| Objetivo | Meta medible |
|---|---|
| RAM en reposo (escritorio completo cargado) | ≤ 450 MB |
| RAM máxima del **sistema** (kernel + servicios + shell + compositor) bajo uso normal | ≤ 1 GB |
| Disco instalado | ≤ 8 GB |
| Arranque en frío (NVMe) | ≤ 6 s hasta escritorio |
| Compatibilidad | Ejecutar binarios PE x64 (`.exe`, `.dll`, `.msi`, `.msix`) sin recompilar |
| Hardware | x86-64 con UEFI, TPM opcional |

**Aclaración honesta sobre el 1 GB:** el tope de 1 GB aplica al *sistema operativo*. Una app como Chrome, Photoshop o un juego AAA consume su propia memoria; ningún SO puede evitarlo. Lo que sí se puede hacer es que el SO nunca sea el que consuma la memoria, y que use mejor la que hay (compresión, deduplicación, páginas compartidas).

---

## 2. Problemas de Windows y cómo se resuelven aquí

| Problema en Windows 11 | Causa raíz | Solución en NimbusOS |
|---|---|---|
| 3–4 GB de RAM en reposo | Cientos de servicios siempre activos, shell XAML/WebView2 residente, telemetría, indexado | Servicios **bajo demanda** (socket/evento activa el servicio), shell nativo sin WebView, sin telemetría |
| Procesos duplicados con DLLs duplicadas | Cada app trae su runtime; WinSxS enorme | Almacén **content-addressable** de DLLs (dedupe por hash) + páginas compartidas copy-on-write |
| Win32k en el kernel (superficie de ataque y fallos) | GDI/USER viven en ring 0 | `win32k` **en modo usuario** (servidor de ventanas aislado) |
| Drivers que causan BSOD | Drivers en ring 0 sin aislamiento | Drivers en **modo usuario por defecto** (UMDF-style) con IOMMU; solo storage/GPU en kernel |
| Actualizaciones lentas que requieren reinicio | DLLs en uso bloqueadas | **Particiones A/B** + actualización atómica, hot-patching de servicios |
| Registro corrupto / lento | Hives monolíticos | Registro respaldado por **B-tree con journaling** y snapshots |
| I/O lento en archivos pequeños | NTFS + filtros de AV en cadena | FS propio (NimbusFS) compatible con semántica NTFS, filtros en un único *minifilter bus* |
| Bloatware y anuncios | Decisiones de producto | Imagen base mínima, sin apps preinstaladas ni anuncios |
| Compatibilidad cuesta memoria | WoW64, capas legacy | **Solo x64 nativo**; legacy 32 bits como paquete opcional |
| Swap/pagefile ineficiente | Paginación a disco | **zram-style**: memoria comprimida (LZ4/Zstd) antes de paginar a disco |

---

## 3. Arquitectura en capas

```
┌──────────────────────────────────────────────────────────────┐
│  Apps x64 (Win32 / .NET / UWP-lite / MSIX / Electron / juegos)│
├──────────────────────────────────────────────────────────────┤
│  Subsistema Win32 (user-mode)                                 │
│  kernel32 · kernelbase · user32 · gdi32 · advapi32 · ws2_32   │
│  COM/OLE · DirectX (D3D11/12 → Vulkan) · WinRT-lite · .NET    │
├──────────────────────────────────────────────────────────────┤
│  Servicios del sistema (bajo demanda)                         │
│  Session Manager · Service Control Mgr · Window Server        │
│  Compositor · Audio · Red · Instalador · Actualizador         │
├──────────────────────────────────────────────────────────────┤
│  ntdll (syscall stubs, loader PE, heap, ETW-lite)             │
├══════════════════════ frontera usuario/kernel ═══════════════┤
│  Núcleo NT-compatible (microkernel híbrido)                   │
│  Object Mgr · Process/Thread · Memory Mgr · I/O Mgr           │
│  Security Ref Monitor · Registry · Cache Mgr · IPC (ALPC)     │
├──────────────────────────────────────────────────────────────┤
│  Drivers kernel mínimos: storage · GPU · red · USB host       │
├──────────────────────────────────────────────────────────────┤
│  HAL + Bootloader UEFI + Hipervisor opcional (VBS-lite)       │
└──────────────────────────────────────────────────────────────┘
```

### 3.1 Núcleo (compatible con semántica NT)
- Mantiene la interfaz de **syscalls NT** (`NtCreateFile`, `NtMapViewOfSection`, etc.) para que `ntdll` y el loader PE funcionen igual que en Windows.
- **Microkernel híbrido:** solo planificador, memoria, IPC, objetos y seguridad en ring 0. El resto (FS de red, audio, USB de alto nivel, ventanas) corre en servicios de usuario.

### 3.2 Gestor de memoria (clave para el límite de 1 GB)
1. **Compresión de memoria** previa a paginación (ratio típico 2–3×).
2. **Deduplicación de páginas** (KSM-style) para DLLs y datos idénticos entre procesos.
3. **Secciones compartidas** para todos los PE: una sola copia de `kernel32`, `user32`, etc.
4. **Working-set trimming agresivo** para procesos en segundo plano y servicios inactivos.
5. **Hibernación de procesos** de UI minimizados (se serializa su estado a zram).
6. **Prefetch adaptativo** limitado a un presupuesto fijo (≤ 64 MB) en lugar de SuperFetch sin tope.
7. **Presupuesto duro por componente** (ver tabla 4).

### 3.3 Compatibilidad x64
- **Loader PE/COFF** completo: imports, delay-load, TLS, SEH x64 (`.pdata`/unwind), CFG, relocations, resources.
- **Win32 API** reimplementada: miles de funciones; prioridad por telemetría de uso real (primero el top 3000 APIs que cubren >99 % de apps).
- **DirectX → Vulkan** (traducción tipo DXVK/VKD3D) para D3D9/10/11/12.
- **.NET / .NET Framework**: se ejecuta el runtime oficial sobre la API Win32 compatible.
- **Instaladores**: MSI (motor propio), MSIX/AppX, Inno/NSIS (son .exe normales).
- **Anti-cheat y DRM de kernel**: son el punto más difícil; se ofrece una interfaz de *attestation* y soporte a proveedores que se integren (ver riesgos).

### 3.4 Gráficos y UI
- **Window Server** en modo usuario (reemplaza win32k) + **compositor** GPU (Vulkan) con presupuesto de 120 MB.
- **Shell nativo** (barra, menú inicio, explorador) escrito en Rust/C++ sin WebView ni XAML pesado.
- Soporte de temas claro/oscuro, DPI mixto, multi-monitor, HDR.

### 3.5 Seguridad
- Modelo de tokens/SIDs/ACLs compatible con NT.
- **Sandboxing por app** (AppContainer-style) y drivers en modo usuario.
- Arranque seguro, TPM 2.0 opcional, cifrado de disco (XTS-AES).
- Actualizaciones firmadas, particiones A/B con rollback.

---

## 4. Presupuesto de memoria (sistema en reposo, escritorio cargado)

| Componente | Presupuesto |
|---|---|
| Kernel + HAL + drivers kernel | 90 MB |
| Cache manager + estructuras de memoria | 60 MB |
| Registro (hives en memoria) | 20 MB |
| Session Manager + SCM + servicios activos | 60 MB |
| Window Server + compositor | 120 MB |
| Shell (barra, inicio, explorador) | 70 MB |
| Audio + red + Bluetooth | 35 MB |
| Seguridad (Defender-lite, firewall) | 40 MB |
| Búferes de GPU del sistema | 80 MB |
| **Total en reposo** | **≈ 575 MB** |
| Margen para picos (indexado, actualización, instalación) | ≈ 425 MB |
| **Techo del sistema** | **1024 MB** |

El cache de archivos y las apps de usuario **no** cuentan en este presupuesto (el cache es reclamable al instante).

---

## 5. Listado completo de archivos del código fuente

Lenguajes: **C/C++ (kernel, drivers)**, **Rust (servicios, shell, FS)**, **ASM x64 (arranque, stubs)**.

```
nimbusos/
├── README.md
├── LICENSE
├── CMakeLists.txt
├── Cargo.toml
├── build/
│   ├── toolchain-x64-msvc-abi.cmake
│   ├── make_iso.py
│   ├── make_vhdx.py
│   ├── sign_image.py
│   ├── ab_partition_layout.json
│   └── ci/
│       ├── build.yml
│       ├── test_boot_qemu.yml
│       └── compat_suite.yml
│
├── boot/
│   ├── uefi/
│   │   ├── bootmgr.c
│   │   ├── bootmgr.h
│   │   ├── loader_kernel.c
│   │   ├── memory_map.c
│   │   ├── secure_boot.c
│   │   ├── tpm_measure.c
│   │   ├── ab_select.c
│   │   ├── graphics_gop.c
│   │   └── entry.asm
│   └── recovery/
│       ├── recovery_env.c
│       └── rollback.c
│
├── hal/
│   ├── hal.h
│   ├── hal_init.c
│   ├── apic.c
│   ├── ioapic.c
│   ├── hpet.c
│   ├── tsc.c
│   ├── acpi/
│   │   ├── acpi_tables.c
│   │   ├── aml_interp.c
│   │   ├── power_states.c
│   │   └── thermal.c
│   ├── pci/
│   │   ├── pci_enum.c
│   │   ├── msi.c
│   │   └── iommu_vtd.c
│   ├── smp.c
│   ├── interrupts.c
│   ├── idt_gdt.c
│   ├── cpu_features.c
│   └── asm/
│       ├── context_switch.asm
│       ├── syscall_entry.asm
│       ├── interrupts_stubs.asm
│       └── cpuid.asm
│
├── kernel/
│   ├── ke/                      # Kernel core: scheduler, sync
│   │   ├── scheduler.c
│   │   ├── scheduler_efficiency_cores.c
│   │   ├── dpc.c
│   │   ├── apc.c
│   │   ├── mutex.c
│   │   ├── event.c
│   │   ├── semaphore.c
│   │   ├── spinlock.c
│   │   ├── timer.c
│   │   ├── wait.c
│   │   ├── exception_dispatch.c
│   │   ├── bugcheck.c
│   │   └── syscall_table.c
│   │
│   ├── mm/                      # Memory manager
│   │   ├── mm_init.c
│   │   ├── physical_alloc.c
│   │   ├── virtual_alloc.c
│   │   ├── page_fault.c
│   │   ├── section_objects.c
│   │   ├── working_set.c
│   │   ├── working_set_trim.c
│   │   ├── zram_compress.c
│   │   ├── zram_lz4.c
│   │   ├── zram_zstd.c
│   │   ├── page_dedupe.c
│   │   ├── cow.c
│   │   ├── pagefile.c
│   │   ├── memory_budget.c        # presupuestos duros por componente
│   │   ├── memory_pressure.c
│   │   ├── large_pages.c
│   │   ├── pool_alloc.c
│   │   ├── kasan_lite.c
│   │   └── numa.c
│   │
│   ├── ob/                      # Object manager
│   │   ├── object_types.c
│   │   ├── object_namespace.c
│   │   ├── handle_table.c
│   │   ├── symlinks.c
│   │   └── refcount.c
│   │
│   ├── ps/                      # Processes & threads
│   │   ├── process.c
│   │   ├── thread.c
│   │   ├── job_objects.c
│   │   ├── process_hibernate.c
│   │   ├── affinity.c
│   │   ├── priority_boost.c
│   │   └── pe_image_load.c
│   │
│   ├── io/                      # I/O manager
│   │   ├── irp.c
│   │   ├── driver_model.c
│   │   ├── umdf_host.c           # drivers en modo usuario
│   │   ├── minifilter_bus.c
│   │   ├── device_tree.c
│   │   ├── pnp.c
│   │   ├── power_manager.c
│   │   ├── async_io_ring.c       # io_uring-style
│   │   └── dma.c
│   │
│   ├── se/                      # Security reference monitor
│   │   ├── token.c
│   │   ├── sid.c
│   │   ├── acl.c
│   │   ├── access_check.c
│   │   ├── privileges.c
│   │   ├── appcontainer.c
│   │   ├── integrity_levels.c
│   │   └── audit.c
│   │
│   ├── cm/                      # Configuration manager (registro)
│   │   ├── registry_core.c
│   │   ├── hive_btree.c
│   │   ├── hive_journal.c
│   │   ├── hive_snapshot.c
│   │   ├── registry_notify.c
│   │   └── registry_virtualization.c
│   │
│   ├── cc/                      # Cache manager
│   │   ├── cache_map.c
│   │   ├── readahead.c
│   │   ├── writeback.c
│   │   └── cache_shrink.c
│   │
│   ├── alpc/                    # IPC
│   │   ├── port.c
│   │   ├── message.c
│   │   ├── shared_view.c
│   │   └── rpc_transport.c
│   │
│   ├── etw/                     # Tracing liviano
│   │   ├── trace_provider.c
│   │   ├── trace_session.c
│   │   └── perf_counters.c
│   │
│   ├── vbs/                     # Aislamiento por hipervisor (opcional)
│   │   ├── hypervisor_lite.c
│   │   ├── secure_kernel.c
│   │   └── credential_guard.c
│   │
│   ├── include/
│   │   ├── nt.h
│   │   ├── ntstatus.h
│   │   ├── ntdef.h
│   │   ├── ntapi.h
│   │   ├── ke.h
│   │   ├── mm.h
│   │   ├── ob.h
│   │   ├── ps.h
│   │   ├── io.h
│   │   ├── se.h
│   │   ├── cm.h
│   │   └── cc.h
│   │
│   └── main.c                   # KiSystemStartup / init
│
├── drivers/
│   ├── storage/
│   │   ├── nvme.c
│   │   ├── ahci.c
│   │   ├── usb_storage.c
│   │   ├── partmgr.c
│   │   ├── volmgr.c
│   │   └── bitlocker_like.c
│   ├── filesystem/
│   │   ├── nimbusfs/            # FS nativo (Rust)
│   │   │   ├── Cargo.toml
│   │   │   └── src/
│   │   │       ├── lib.rs
│   │   │       ├── superblock.rs
│   │   │       ├── btree.rs
│   │   │       ├── extents.rs
│   │   │       ├── journal.rs
│   │   │       ├── snapshots.rs
│   │   │       ├── dedupe.rs
│   │   │       ├── compression.rs
│   │   │       ├── ntfs_semantics.rs   # ACLs, ADS, hard links, reparse points
│   │   │       ├── encryption.rs
│   │   │       └── trim.rs
│   │   ├── ntfs_ro/             # lectura de NTFS existente
│   │   ├── fat32_exfat/
│   │   ├── iso9660_udf/
│   │   └── smb_client/
│   ├── gpu/
│   │   ├── gpu_core.c
│   │   ├── vulkan_kmd.c
│   │   ├── intel/
│   │   ├── amd/
│   │   ├── nvidia_open/
│   │   └── framebuffer_fallback.c
│   ├── network/
│   │   ├── tcpip/
│   │   │   ├── ip4.c
│   │   │   ├── ip6.c
│   │   │   ├── tcp.c
│   │   │   ├── udp.c
│   │   │   ├── quic.c
│   │   │   ├── dns_client.c
│   │   │   └── firewall_core.c
│   │   ├── nic_intel.c
│   │   ├── nic_realtek.c
│   │   ├── nic_virtio.c
│   │   └── wifi/
│   │       ├── wifi_core.c
│   │       ├── wpa3.c
│   │       └── fw_loader.c
│   ├── usb/
│   │   ├── xhci.c
│   │   ├── usb_core.c
│   │   ├── hid.c
│   │   └── hub.c
│   ├── input/
│   │   ├── ps2.c
│   │   ├── i2c_hid.c
│   │   ├── precision_touchpad.c
│   │   └── touchscreen.c
│   ├── audio/                   # UMDF (modo usuario)
│   │   ├── hda.c
│   │   ├── usb_audio.c
│   │   └── bluetooth_a2dp.c
│   ├── bluetooth/
│   ├── power/
│   │   ├── battery.c
│   │   ├── lid_button.c
│   │   └── cpu_freq.c
│   ├── virtio/
│   └── tpm/
│       └── tpm2.c
│
├── userland/
│   ├── ntdll/
│   │   ├── ntdll.def
│   │   ├── syscall_stubs.asm
│   │   ├── loader/
│   │   │   ├── ldr_init.c
│   │   │   ├── ldr_pe_parse.c
│   │   │   ├── ldr_imports.c
│   │   │   ├── ldr_delayload.c
│   │   │   ├── ldr_relocs.c
│   │   │   ├── ldr_tls.c
│   │   │   ├── ldr_shared_store.c     # almacén dedupe de DLLs
│   │   │   └── ldr_manifest.c
│   │   ├── heap/
│   │   │   ├── heap_core.c
│   │   │   ├── heap_lowfrag.c
│   │   │   └── heap_segment.c
│   │   ├── rtl/
│   │   │   ├── rtl_string.c
│   │   │   ├── rtl_unwind_x64.c
│   │   │   ├── rtl_seh.c
│   │   │   ├── rtl_critsec.c
│   │   │   └── rtl_threadpool.c
│   │   └── etw_user.c
│   │
│   ├── win32/
│   │   ├── kernel32/
│   │   │   ├── file.c
│   │   │   ├── process.c
│   │   │   ├── thread.c
│   │   │   ├── sync.c
│   │   │   ├── memory.c
│   │   │   ├── console.c
│   │   │   ├── time.c
│   │   │   ├── module.c
│   │   │   ├── fiber.c
│   │   │   ├── pipe.c
│   │   │   ├── locale_nls.c
│   │   │   └── kernel32.def
│   │   ├── kernelbase/
│   │   ├── user32/
│   │   │   ├── window.c
│   │   │   ├── message.c
│   │   │   ├── input.c
│   │   │   ├── clipboard.c
│   │   │   ├── menu.c
│   │   │   ├── dialog.c
│   │   │   ├── hooks.c
│   │   │   ├── class.c
│   │   │   ├── cursor_icon.c
│   │   │   ├── dpi.c
│   │   │   └── user32.def
│   │   ├── gdi32/
│   │   │   ├── dc.c
│   │   │   ├── bitmap.c
│   │   │   ├── brush_pen.c
│   │   │   ├── font.c
│   │   │   ├── path.c
│   │   │   ├── region.c
│   │   │   ├── print.c
│   │   │   └── gdi32.def
│   │   ├── gdiplus/
│   │   ├── advapi32/
│   │   │   ├── registry.c
│   │   │   ├── security.c
│   │   │   ├── services.c
│   │   │   ├── crypto.c
│   │   │   └── eventlog.c
│   │   ├── shell32/
│   │   ├── shlwapi/
│   │   ├── comctl32/
│   │   ├── comdlg32/
│   │   ├── ole32/
│   │   ├── oleaut32/
│   │   ├── rpcrt4/
│   │   ├── ws2_32/
│   │   ├── wininet/
│   │   ├── winhttp/
│   │   ├── crypt32/
│   │   ├── bcrypt/
│   │   ├── ncrypt/
│   │   ├── secur32/
│   │   ├── setupapi/
│   │   ├── cfgmgr32/
│   │   ├── dwmapi/
│   │   ├── uxtheme/
│   │   ├── version/
│   │   ├── psapi/
│   │   ├── dbghelp/
│   │   ├── msvcrt/
│   │   ├── ucrtbase/
│   │   ├── vcruntime/
│   │   ├── msvcp/
│   │   ├── combase/
│   │   ├── winrt/               # WinRT-lite
│   │   ├── mfplat/              # Media Foundation
│   │   ├── xaudio2/
│   │   ├── dsound/
│   │   ├── mmdevapi/
│   │   ├── dinput8/
│   │   ├── xinput/
│   │   └── winmm/
│   │
│   ├── directx/
│   │   ├── dxgi/
│   │   ├── d3d9_to_vk/
│   │   ├── d3d10_to_vk/
│   │   ├── d3d11_to_vk/
│   │   ├── d3d12_to_vk/
│   │   ├── dxcompiler/
│   │   ├── d2d1/
│   │   ├── dwrite/
│   │   └── wic/
│   │
│   ├── dotnet/
│   │   ├── hostfxr_shim/
│   │   ├── clr_pal/             # capa de plataforma para CoreCLR
│   │   └── netfx_compat/        # .NET Framework 4.x
│   │
│   └── win32k_server/           # sustituto de win32k, en modo usuario
│       ├── ws_main.rs
│       ├── window_manager.rs
│       ├── input_router.rs
│       ├── focus_z_order.rs
│       ├── gdi_backend.rs
│       ├── hit_testing.rs
│       ├── session_desktop.rs
│       ├── clipboard_server.rs
│       └── accessibility.rs
│
├── services/                    # Servicios del sistema (Rust)
│   ├── smss/                    # Session Manager
│   ├── csrss_lite/
│   ├── wininit/
│   ├── winlogon/
│   ├── lsass_lite/              # autenticación
│   ├── scm/                     # Service Control Manager (activación bajo demanda)
│   │   ├── scm_main.rs
│   │   ├── on_demand_trigger.rs
│   │   ├── service_db.rs
│   │   └── resource_limits.rs
│   ├── compositor/
│   │   ├── compositor_main.rs
│   │   ├── vulkan_backend.rs
│   │   ├── damage_tracking.rs
│   │   ├── animations.rs
│   │   ├── hdr_color.rs
│   │   └── multi_monitor.rs
│   ├── audio_engine/
│   ├── network_manager/
│   │   ├── dhcp.rs
│   │   ├── wifi_profiles.rs
│   │   ├── vpn.rs
│   │   └── proxy.rs
│   ├── update_service/
│   │   ├── check.rs
│   │   ├── delta_download.rs
│   │   ├── apply_ab.rs
│   │   ├── hotpatch.rs
│   │   └── rollback.rs
│   ├── installer_service/
│   │   ├── msi_engine/
│   │   │   ├── msi_parser.rs
│   │   │   ├── msi_transactions.rs
│   │   │   ├── custom_actions.rs
│   │   │   └── msi_rollback.rs
│   │   ├── msix_engine/
│   │   │   ├── package_reader.rs
│   │   │   ├── manifest.rs
│   │   │   ├── containerization.rs
│   │   │   └── dependencies.rs
│   │   ├── legacy_exe_monitor.rs    # detecta instaladores .exe y registra cambios
│   │   └── uninstall_tracker.rs
│   ├── defender_lite/
│   │   ├── realtime_scan.rs
│   │   ├── signatures.rs
│   │   ├── cloud_lookup.rs
│   │   └── behavior_monitor.rs
│   ├── firewall/
│   ├── search_indexer/          # con tope de memoria y CPU
│   ├── print_spooler/
│   ├── time_service/
│   ├── power_service/
│   ├── crypto_service/
│   ├── eventlog/
│   ├── task_scheduler/
│   ├── bluetooth_service/
│   ├── geolocation/
│   └── memory_governor/         # aplica presupuestos y hiberna procesos
│       ├── governor_main.rs
│       ├── budget_policy.rs
│       ├── process_hibernator.rs
│       └── pressure_events.rs
│
├── shell/                       # Interfaz de usuario (Rust)
│   ├── taskbar/
│   │   ├── taskbar.rs
│   │   ├── tray.rs
│   │   ├── notifications.rs
│   │   └── jump_lists.rs
│   ├── start_menu/
│   │   ├── start.rs
│   │   ├── app_index.rs
│   │   └── search_ui.rs
│   ├── explorer/
│   │   ├── explorer_main.rs
│   │   ├── file_view.rs
│   │   ├── nav_pane.rs
│   │   ├── context_menu.rs
│   │   ├── shell_extensions_host.rs   # aislado en proceso aparte
│   │   ├── thumbnails.rs
│   │   └── tabs.rs
│   ├── settings_app/
│   │   ├── system.rs
│   │   ├── display.rs
│   │   ├── network.rs
│   │   ├── apps.rs
│   │   ├── accounts.rs
│   │   ├── privacy.rs
│   │   ├── update.rs
│   │   └── memory_monitor.rs
│   ├── task_manager/
│   ├── lock_screen/
│   ├── widgets_lite/            # opcional, apagado por defecto
│   ├── snap_layouts/
│   ├── virtual_desktops/
│   ├── quick_settings/
│   ├── run_dialog/
│   ├── themes/
│   │   ├── light.toml
│   │   ├── dark.toml
│   │   └── high_contrast.toml
│   └── assets/
│       ├── icons/
│       ├── fonts/
│       └── wallpapers/
│
├── tools/                       # Utilidades de línea de comandos
│   ├── cmd/
│   ├── powershell_compat/
│   ├── terminal/
│   ├── winget_like/             # gestor de paquetes
│   ├── diskpart_like/
│   ├── sfc_like/
│   ├── reg/
│   ├── sc/
│   ├── tasklist_taskkill/
│   ├── netsh_like/
│   ├── ping_tracert/
│   ├── chkdsk_like/
│   └── perfmon_lite/
│
├── compat/                      # Capa de compatibilidad
│   ├── app_shims/               # parches por aplicación (como AppCompat)
│   │   ├── shim_db.sqlite
│   │   ├── shim_engine.c
│   │   └── known_apps/
│   ├── drm_attestation/
│   │   ├── attestation_api.c
│   │   └── provider_interface.h
│   ├── anticheat_bridge/
│   │   ├── kernel_callbacks_emul.c
│   │   └── provider_interface.h
│   ├── legacy32/                # paquete OPCIONAL (WoW64-lite)
│   │   ├── wow64_core.c
│   │   ├── wow64_thunks/
│   │   └── syswow64_dlls/
│   └── linux_wsl_lite/          # opcional: ejecutar binarios ELF
│
├── sdk/
│   ├── include/                 # windows.h, headers públicos
│   ├── lib/
│   ├── docs/
│   └── samples/
│
├── tests/
│   ├── kernel/
│   │   ├── test_scheduler.c
│   │   ├── test_mm_zram.c
│   │   ├── test_mm_dedupe.c
│   │   ├── test_ob_handles.c
│   │   └── test_registry.c
│   ├── win32_conformance/       # compara comportamiento con Windows real
│   ├── app_compat/
│   │   ├── top_1000_apps.csv
│   │   ├── run_compat.py
│   │   └── results/
│   ├── memory_budget/
│   │   ├── idle_ram_test.py
│   │   └── stress_1gb_test.py
│   ├── fuzzing/
│   │   ├── fuzz_syscalls.c
│   │   ├── fuzz_pe_loader.c
│   │   └── fuzz_registry.c
│   └── boot/
│       └── qemu_boot_test.sh
│
└── docs/
    ├── architecture.md
    ├── memory_budget.md
    ├── compat_strategy.md
    ├── driver_model.md
    ├── security_model.md
    ├── update_ab_design.md
    └── contributing.md
```

---

## 6. Flujo de arranque

1. **UEFI** → `bootmgr` verifica firma, mide en TPM, elige partición A o B.
2. Carga `kernel` + `hal` + drivers de storage → `main.c` (inicialización de `ke`, `mm`, `ob`, `ps`, `io`, `se`, `cm`).
3. Arranca `smss` → inicia `csrss_lite`, `wininit`, `scm`.
4. `scm` activa **solo** los servicios necesarios para login (≈ 12 en vez de ≈ 200).
5. `winlogon` → `lsass_lite` → sesión de usuario.
6. `win32k_server` + `compositor` + `shell` en paralelo.
7. El resto de servicios se activa por evento (red conectada, USB insertado, impresión, etc.).

---

## 7. Cómo se ejecuta una app x64

```
setup.exe / app.exe
  │
  ▼
ps/pe_image_load.c  → mapea secciones PE (compartidas si ya están en el almacén)
  │
  ▼
ntdll/loader  → resuelve imports → kernel32/user32/... ya mapeadas (COW)
  │
  ▼
Llamadas Win32 → ntdll → syscall → kernel NT-compatible
  │                       └→ ALPC → Window Server / servicios
  ▼
DirectX → traducción a Vulkan → driver GPU
```

---

## 8. Hoja de ruta sugerida

| Fase | Contenido | Resultado |
|---|---|---|
| 0 | Bootloader UEFI + HAL + kernel mínimo (scheduler, MM) | Arranca en QEMU, imprime en consola |
| 1 | Object manager, procesos, syscalls NT, `ntdll`, loader PE | Corre un `.exe` de consola x64 |
| 2 | `kernel32`/`kernelbase`, registro, NimbusFS, red básica | Corre CLI de terceros (git, python) |
| 3 | Window Server, `user32`/`gdi32`, shell mínimo | Apps GUI simples (Notepad, 7-Zip) |
| 4 | DirectX→Vulkan, audio, COM/OLE, .NET | Navegadores, Office-like, juegos |
| 5 | Instalador MSI/MSIX, actualizaciones A/B, Defender-lite | Instalación de apps comerciales |
| 6 | Memory governor, compresión, dedupe, ajuste fino | Cumplir ≤ 1 GB |
| 7 | Anti-cheat/DRM bridge, certificación de drivers | Compatibilidad amplia |

---

## 9. Riesgos y límites reales

1. **"Todas las apps x64" es un objetivo asintótico.** Se logra ≈ 95 % con Win32 + DirectX bien implementados; el último 5 % (drivers de kernel propietarios, anti-cheat, DRM, software con APIs internas no documentadas) requiere acuerdos con terceros o shims por app.
2. **Drivers de hardware:** Windows tiene décadas de drivers de fabricantes. Habrá que escribir los principales y ofrecer un host UMDF para portar drivers existentes.
3. **Documentación incompleta:** algunas APIs de Windows no están documentadas; se necesita pruebas de conformidad contra Windows real (caja negra).
4. **Aspectos legales:** clean-room estricto, sin copiar código ni binarios de Microsoft; revisar patentes y licencias (por ejemplo códecs y fuentes).
5. **Esfuerzo:** un equipo de ~50–100 ingenieros tardaría años. ReactOS lleva más de 25 años en un alcance similar; este diseño reduce el alcance eligiendo solo x64, Rust para servicios y reutilizando traducciones tipo DXVK.
6. **El tope de 1 GB** es del sistema; apps pesadas necesitarán RAM adicional, y el sistema tiene que degradar con elegancia (compresión → hibernación de procesos → paginación) en lugar de fallar.

---

## 10. Resumen

NimbusOS combina un **kernel NT-compatible reducido**, **win32k en modo usuario**, **servicios bajo demanda**, **memoria comprimida y deduplicada** y un **shell nativo sin WebView** para bajar el consumo base de ≈ 3–4 GB (Windows 11) a ≈ 575 MB en reposo, manteniendo la ejecución de binarios PE x64 mediante una reimplementación fiel de las APIs Win32/NT y una traducción de DirectX a Vulkan.
