#!/usr/bin/env bash
# NimbusOS — reconstruir la ISO arrancable desde el código fuente.
# Uso: ./build/rebuild_iso.sh
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
echo "NimbusOS: reconstruyendo ISO..."
python3 build/make_iso.py -o "$ROOT/out/nimbusos.iso"
echo "Listo: $ROOT/out/nimbusos.iso"
echo "Probar: ./tests/boot/test_qemu_boot.sh"
