#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
NimbusOS — genera una ISO arrancable (Multiboot2 + GRUB) desde el código fuente.

Flujo:
  1. Compila boot/freestanding → out/kernel.elf  (nasm + gcc -ffreestanding + ld)
  2. Prepara un árbol ISO con grub.cfg y el kernel
  3. Invoca grub-mkrescue (xorriso) → out/nimbusos.iso

Uso:
  python3 build/make_iso.py
  python3 build/make_iso.py -o /ruta/nimbusos.iso

Requisitos (apt): nasm, gcc, binutils, grub-pc-bin, grub-common, xorriso, mtools
Fallback documentado: si faltara grub-efi, usamos solo i386-pc (BIOS) que QEMU
soporta sin OVMF. Marca: NimbusOS (clean-room, no Windows).
"""
from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
FREESTANDING = ROOT / "boot" / "freestanding"
DEFAULT_OUT = ROOT / "out" / "nimbusos.iso"
KERNEL_ELF = ROOT / "out" / "kernel.elf"


def run(cmd: list[str], **kw) -> None:
    print("+", " ".join(cmd), flush=True)
    subprocess.check_call(cmd, **kw)


def build_kernel() -> Path:
    """Compila el kernel freestanding Multiboot2."""
    outdir = ROOT / "out"
    outdir.mkdir(parents=True, exist_ok=True)
    run(["make", "-C", str(FREESTANDING), f"OUTDIR={outdir}", "all"])
    if not KERNEL_ELF.is_file():
        raise SystemExit(f"error: no se generó {KERNEL_ELF}")
    return KERNEL_ELF


def make_iso(kernel: Path, output: Path) -> Path:
    """Empaqueta kernel + GRUB en una ISO bootable."""
    output = output.resolve()
    output.parent.mkdir(parents=True, exist_ok=True)

    grub_mkrescue = shutil.which("grub-mkrescue")
    if not grub_mkrescue:
        raise SystemExit(
            "error: grub-mkrescue no está en PATH. "
            "Instala: sudo apt install grub-pc-bin grub-common xorriso mtools"
        )

    with tempfile.TemporaryDirectory(prefix="nimbusos-iso-") as tmp:
        iso_root = Path(tmp) / "iso"
        boot_dir = iso_root / "boot"
        grub_dir = boot_dir / "grub"
        grub_dir.mkdir(parents=True)

        shutil.copy2(kernel, boot_dir / "kernel.elf")
        cfg_src = FREESTANDING / "grub" / "grub.cfg"
        shutil.copy2(cfg_src, grub_dir / "grub.cfg")

        # grub-mkrescue genera BIOS (i386-pc) y, si hay módulos, también EFI.
        cmd = [
            grub_mkrescue,
            "-o", str(output),
            str(iso_root),
            "--",
            "-volid", "NIMBUSOS",
        ]
        run(cmd)

    size = output.stat().st_size
    print(f"NimbusOS ISO lista: {output} ({size} bytes)", flush=True)
    return output


def main() -> int:
    ap = argparse.ArgumentParser(description="Build NimbusOS bootable ISO")
    ap.add_argument("-o", "--output", type=Path, default=DEFAULT_OUT,
                    help=f"ruta ISO (default: {DEFAULT_OUT})")
    ap.add_argument("--skip-build", action="store_true",
                    help="reutilizar out/kernel.elf existente")
    args = ap.parse_args()

    if args.skip_build and KERNEL_ELF.is_file():
        kernel = KERNEL_ELF
        print(f"Reutilizando kernel: {kernel}")
    else:
        kernel = build_kernel()

    make_iso(kernel, args.output)
    return 0


if __name__ == "__main__":
    sys.exit(main())
