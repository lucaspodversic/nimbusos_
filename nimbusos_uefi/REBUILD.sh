#!/usr/bin/env bash
# NimbusOS — reconstruir imágenes BIOS + UEFI desde este zip y probarlas en QEMU.
# Requisitos: sudo apt install nasm gcc binutils grub-pc-bin grub-common xorriso mtools \
#             dosfstools gdisk gnu-efi ovmf qemu-system-x86
set -euo pipefail
cd "$(dirname "$0")/nimbusos"
./build/rebuild_all.sh "$@"
