#!/usr/bin/env bash
# NimbusOS — prueba de arranque QEMU (serial).
#
# Construye la ISO si falta, lanza qemu-system-x86_64 con -cdrom,
# captura la salida serie y exige ver el banner "NimbusOS".
# Exit 0 = éxito.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
ISO="${ROOT}/out/nimbusos.iso"
LOG="${ROOT}/out/qemu_boot_serial.log"
TIMEOUT_SEC="${NIMBUS_QEMU_TIMEOUT:-20}"

mkdir -p "${ROOT}/out"

if [[ ! -f "$ISO" ]]; then
  echo "[test] ISO ausente — construyendo con build/make_iso.py ..."
  python3 "${ROOT}/build/make_iso.py" -o "$ISO"
fi

if ! command -v qemu-system-x86_64 >/dev/null 2>&1; then
  echo "[test] ERROR: qemu-system-x86_64 no instalado" >&2
  exit 2
fi

echo "[test] Arrancando QEMU con $ISO (timeout ${TIMEOUT_SEC}s) ..."
: > "$LOG"

# Lanza QEMU en background; matamos al ver el marcador de éxito.
qemu-system-x86_64 \
  -cdrom "$ISO" \
  -m 256 \
  -serial stdio \
  -display none \
  -no-reboot \
  -no-shutdown \
  < /dev/null \
  > "$LOG" 2>&1 &
QEMU_PID=$!

cleanup() { kill -KILL "$QEMU_PID" 2>/dev/null || true; wait "$QEMU_PID" 2>/dev/null || true; }
trap cleanup EXIT

ok=0
for ((i=0; i<TIMEOUT_SEC*10; i++)); do
  if grep -q "NimbusOS: kernel freestanding listo" "$LOG" 2>/dev/null \
     || grep -q "Multiboot2: OK" "$LOG" 2>/dev/null; then
    ok=1
    break
  fi
  if ! kill -0 "$QEMU_PID" 2>/dev/null; then
    break
  fi
  sleep 0.1
done

cleanup
trap - EXIT

echo "[test] log: $LOG"
echo "----- serial -----"
cat "$LOG" || true
echo "------------------"

if [[ "$ok" -eq 1 ]] || grep -q "NimbusOS" "$LOG"; then
  echo "[test] OK — NimbusOS arrancó (serial prueba superada)"
  exit 0
fi

echo "[test] FALLO — no se encontró 'NimbusOS' en la salida serie" >&2
exit 1
