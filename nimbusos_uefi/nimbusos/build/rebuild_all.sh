#!/usr/bin/env bash
# NimbusOS — reconstruye TODAS las imágenes de arranque y ejecuta las pruebas QEMU.
#   BIOS : out/nimbusos.iso            (Multiboot2 + GRUB)
#   UEFI : out/nimbusos_uefi.img/.iso  (BOOTX64.EFI + OVMF)
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
python3 build/make_iso.py  -o "$ROOT/out/nimbusos.iso"
python3 build/make_uefi.py
if [[ "${1:-}" != "--no-test" ]]; then
  ./tests/boot/test_qemu_boot.sh
  ./tests/boot/test_qemu_uefi.sh
  ./tests/boot/test_qemu_uefi.sh --iso
fi
echo "NimbusOS: imágenes listas en $ROOT/out"
