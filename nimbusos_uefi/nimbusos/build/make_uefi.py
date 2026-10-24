#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
NimbusOS — genera las imágenes de arranque UEFI desde el código fuente.

Salidas (en out/):
  * nimbusos_uefi.img  — disco GPT con una ESP FAT32 (para -drive / USB)
  * nimbusos_uefi.iso  — ISO El Torito con imagen EFI (para -cdrom / óptico)

Contenido de la ESP (en ambas):
  /EFI/BOOT/BOOTX64.EFI   cargador UEFI (boot/uefi/app/bootx64.c, gnu-efi)
  /NIMBUS/KERNEL.ELF      kernel freestanding (boot/freestanding, el mismo
                          binario que arranca GRUB por Multiboot2)

Pasos:
  1. make boot/freestanding  → out/kernel.elf
  2. make boot/uefi/app      → out/BOOTX64.EFI
  3. mkfs.fat + mtools (mmd/mcopy) para poblar la FAT sin montar (sin root)
  4. sgdisk crea la tabla GPT (partición tipo EF00 "EFI System")
  5. dd vuelca la FAT32 en el offset de la partición 1 (LBA 2048)
  6. xorriso crea la ISO UEFI con una FAT12 pequeña como imagen El Torito

Requisitos apt: gnu-efi gcc binutils nasm mtools dosfstools gdisk xorriso
Marca: NimbusOS (clean-room, no Windows).
"""
from __future__ import annotations

import argparse
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "out"
FREESTANDING = ROOT / "boot" / "freestanding"
UEFI_APP = ROOT / "boot" / "uefi" / "app"
KERNEL = OUT / "kernel.elf"
EFI_APP = OUT / "BOOTX64.EFI"

SECTOR = 512
# Geometría del disco GPT: 64 MiB totales; ESP en LBA 2048 con 62 MiB.
# 62 MiB con clústeres de 512 B ≈ 126 000 clústeres → FAT32 válido (> 65 525).
DISK_MIB = 64
ESP_START_LBA = 2048
ESP_SECTORS = 62 * 1024 * 1024 // SECTOR


def run(cmd: list[str], **kw) -> None:
    print("+", " ".join(str(c) for c in cmd), flush=True)
    subprocess.check_call([str(c) for c in cmd], **kw)


def need(tool: str, pkg: str) -> str:
    path = shutil.which(tool) or shutil.which(tool, path="/usr/sbin:/sbin")
    if not path:
        raise SystemExit(f"error: falta '{tool}'. Instala: sudo apt install {pkg}")
    return path


def build_binaries() -> None:
    OUT.mkdir(parents=True, exist_ok=True)
    run(["make", "-C", FREESTANDING, f"OUTDIR={OUT}", "all"])
    run(["make", "-C", UEFI_APP, f"OUTDIR={OUT}", "all"])
    for f in (KERNEL, EFI_APP):
        if not f.is_file():
            raise SystemExit(f"error: no se generó {f}")


def populate_fat(fat: Path) -> None:
    """Copia BOOTX64.EFI y KERNEL.ELF dentro de una imagen FAT con mtools."""
    env_img = f"{fat}"
    for d in ("::/EFI", "::/EFI/BOOT", "::/NIMBUS"):
        run(["mmd", "-i", env_img, d])
    run(["mcopy", "-i", env_img, EFI_APP, "::/EFI/BOOT/BOOTX64.EFI"])
    run(["mcopy", "-i", env_img, KERNEL, "::/NIMBUS/KERNEL.ELF"])
    # startup.nsh: si el usuario cae en la UEFI Shell, arranca NimbusOS igual.
    with tempfile.NamedTemporaryFile("w", suffix=".nsh", delete=False) as t:
        t.write("@echo -off\r\nfs0:\r\n\\EFI\\BOOT\\BOOTX64.EFI\r\n")
        nsh = t.name
    run(["mcopy", "-i", env_img, nsh, "::/startup.nsh"])
    Path(nsh).unlink()
    run(["mdir", "-i", env_img, "-/", "::/"])


def make_disk_image(out_img: Path) -> Path:
    mkfs = need("mkfs.fat", "dosfstools")
    sgdisk = need("sgdisk", "gdisk")
    need("mcopy", "mtools")

    with tempfile.TemporaryDirectory(prefix="nimbusos-esp-") as tmp:
        fat = Path(tmp) / "esp.fat"
        # FAT32, 1 sector por clúster, etiqueta del volumen
        run([mkfs, "-F", "32", "-s", "1", "-n", "NIMBUS_ESP", "-C", fat, ESP_SECTORS * SECTOR // 1024])
        populate_fat(fat)

        out_img.unlink(missing_ok=True)
        with open(out_img, "wb") as f:
            f.truncate(DISK_MIB * 1024 * 1024)
        end_lba = ESP_START_LBA + ESP_SECTORS - 1
        run([sgdisk, "--clear",
             f"--new=1:{ESP_START_LBA}:{end_lba}",
             "--typecode=1:ef00",
             "--change-name=1:NimbusOS ESP",
             out_img])
        # Volcar la FAT32 dentro de la partición 1 sin truncar el disco
        with open(fat, "rb") as src, open(out_img, "r+b") as dst:
            dst.seek(ESP_START_LBA * SECTOR)
            shutil.copyfileobj(src, dst)
        run([sgdisk, "--print", out_img])
    print(f"NimbusOS UEFI disk: {out_img} ({out_img.stat().st_size} bytes)", flush=True)
    return out_img


def make_uefi_iso(out_iso: Path) -> Path:
    """ISO El Torito (plataforma EFI) con una FAT12 de 4 MiB como efiboot.img."""
    xorriso = need("xorriso", "xorriso")
    mkfs = need("mkfs.fat", "dosfstools")
    with tempfile.TemporaryDirectory(prefix="nimbusos-uefiiso-") as tmp:
        root = Path(tmp) / "iso"
        (root / "EFI" / "BOOT").mkdir(parents=True)
        (root / "NIMBUS").mkdir()
        shutil.copy2(EFI_APP, root / "EFI" / "BOOT" / "BOOTX64.EFI")
        shutil.copy2(KERNEL, root / "NIMBUS" / "KERNEL.ELF")
        efiboot = root / "efiboot.img"
        run([mkfs, "-F", "12", "-n", "NIMBUS_EFI", "-C", efiboot, 4096])
        populate_fat(efiboot)
        out_iso.unlink(missing_ok=True)
        run([xorriso, "-as", "mkisofs", "-V", "NIMBUSOS_UEFI", "-R", "-J",
             "-e", "efiboot.img", "-no-emul-boot", "-isohybrid-gpt-basdat",
             "-o", out_iso, root])
    print(f"NimbusOS UEFI ISO: {out_iso} ({out_iso.stat().st_size} bytes)", flush=True)
    return out_iso


def main() -> int:
    ap = argparse.ArgumentParser(description="Build NimbusOS UEFI images (GPT/ESP + ISO)")
    ap.add_argument("--img", type=Path, default=OUT / "nimbusos_uefi.img")
    ap.add_argument("--iso", type=Path, default=OUT / "nimbusos_uefi.iso")
    ap.add_argument("--no-iso", action="store_true", help="solo la imagen de disco GPT")
    ap.add_argument("--skip-build", action="store_true", help="reutilizar out/kernel.elf y out/BOOTX64.EFI")
    args = ap.parse_args()

    if not (args.skip_build and KERNEL.is_file() and EFI_APP.is_file()):
        build_binaries()
    make_disk_image(args.img.resolve())
    if not args.no_iso:
        make_uefi_iso(args.iso.resolve())
    return 0


if __name__ == "__main__":
    sys.exit(main())
