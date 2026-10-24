# Estrategia de compatibilidad

1. Loader PE/COFF x64 completo (imports, TLS, unwind).
2. Top APIs Win32 reimplementadas (prioridad por uso real).
3. DirectX → Vulkan (capas `d3d*_to_vk`).
4. .NET: hostfxr + PAL sobre Win32 compatible.
5. MSI/MSIX propios; instaladores Inno/NSIS como PE normales.
6. Anti-cheat/DRM: puente de attestation (sin promesas de kernel AC propietarios).
