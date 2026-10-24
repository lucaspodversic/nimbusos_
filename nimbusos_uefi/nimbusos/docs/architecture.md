# Arquitectura NimbusOS

Clean-room, estilo Win11/12, techo de sistema ≤ 1 GB RAM.

## Capas
1. Apps PE x64
2. Win32 user-mode (`kernel32`, `user32`, `gdi32`, `advapi32`, DX→Vulkan)
3. Servicios bajo demanda (SCM)
4. `ntdll` (loader PE, heap, RTL)
5. Núcleo NT-compatible (objetos, MM con zram/dedupe, I/O, seguridad, registro)
6. Drivers mínimos + HAL + UEFI

## Diferencias clave vs Windows
- `win32k` en **modo usuario**
- Servicios **OnDemand** por defecto
- NimbusFS con journal + snapshots
- Presupuestos duros por componente (`memory_budget` / Memory Governor)
