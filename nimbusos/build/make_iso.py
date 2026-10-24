#!/usr/bin/env python3
"""NimbusOS — genera una ISO UEFI de arranque (esqueleto fase 1)."""
import argparse, pathlib, sys

def main():
    ap = argparse.ArgumentParser(description="Build NimbusOS bootable ISO")
    ap.add_argument("-o", "--output", default="nimbusos.iso")
    ap.add_argument("--kernel", default="out/kernel.elf")
    args = ap.parse_args()
    out = pathlib.Path(args.output)
    out.write_bytes(b"NIMBUSOS_ISO_PLACEHOLDER_v1\n")
    print(f"Wrote placeholder ISO {out}")
    return 0

if __name__ == "__main__":
    sys.exit(main())
