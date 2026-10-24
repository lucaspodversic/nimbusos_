#!/usr/bin/env bash
# =============================================================================
# NimbusOS — prueba de arranque UEFI en QEMU + OVMF (salida serie).
#
# Qué comprueba (exit 0 solo si TODO se cumple):
#   1. OVMF carga /EFI/BOOT/BOOTX64.EFI desde la ESP (disco GPT o ISO).
#   2. BOOTX64.EFI llama a ExitBootServices y salta al kernel.
#   3. El kernel imprime el banner "NimbusOS" e identifica la ruta UEFI.
#   4. El shell serie responde: le enviamos "about" por la UART y
#      esperamos la línea "Boot: UEFI" en la respuesta.
#
# Uso:
#   tests/boot/test_qemu_uefi.sh            # disco GPT (out/nimbusos_uefi.img)
#   tests/boot/test_qemu_uefi.sh --iso      # ISO UEFI (out/nimbusos_uefi.iso)
#   NIMBUS_QEMU_TIMEOUT=40 tests/boot/test_qemu_uefi.sh
# Firmware: OVMF_CODE/OVMF_VARS (apt: ovmf) vía -drive if=pflash. Se trabaja
# sobre una COPIA de OVMF_VARS para no ensuciar el fichero del sistema.
# =============================================================================
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="${ROOT}/out"
MODE="img"
[[ "${1:-}" == "--iso" ]] && MODE="iso"
IMG="${OUT}/nimbusos_uefi.img"
ISO="${OUT}/nimbusos_uefi.iso"
LOG="${OUT}/qemu_uefi_serial_${MODE}.log"
TIMEOUT_SEC="${NIMBUS_QEMU_TIMEOUT:-40}"

# --- Localizar OVMF (rutas de Debian/Ubuntu/Fedora) ---------------------------
OVMF_CODE="" ; OVMF_VARS_SRC=""
for c in /usr/share/OVMF/OVMF_CODE_4M.fd /usr/share/OVMF/OVMF_CODE.fd \
         /usr/share/edk2/ovmf/OVMF_CODE.fd /usr/share/qemu/OVMF_CODE.fd; do
  [[ -f "$c" ]] && { OVMF_CODE="$c"; break; }
done
for v in /usr/share/OVMF/OVMF_VARS_4M.fd /usr/share/OVMF/OVMF_VARS.fd \
         /usr/share/edk2/ovmf/OVMF_VARS.fd /usr/share/qemu/OVMF_VARS.fd; do
  [[ -f "$v" ]] && { OVMF_VARS_SRC="$v"; break; }
done
if [[ -z "$OVMF_CODE" || -z "$OVMF_VARS_SRC" ]]; then
  echo "[uefi] ERROR: OVMF no encontrado (sudo apt install ovmf)" >&2; exit 2
fi
command -v qemu-system-x86_64 >/dev/null || { echo "[uefi] ERROR: falta qemu-system-x86" >&2; exit 2; }

# --- Construir imágenes si faltan --------------------------------------------
if [[ ! -f "$IMG" || ! -f "$ISO" ]]; then
  echo "[uefi] Imágenes ausentes — python3 build/make_uefi.py"
  python3 "${ROOT}/build/make_uefi.py"
fi

WORK="$(mktemp -d)"
VARS="${WORK}/OVMF_VARS.fd"
cp "$OVMF_VARS_SRC" "$VARS"
FIFO="${WORK}/serial_in"
mkfifo "$FIFO"

if [[ "$MODE" == "iso" ]]; then
  MEDIA=(-cdrom "$ISO")
else
  MEDIA=(-drive "format=raw,file=${IMG},if=ide,media=disk")
fi

echo "[uefi] OVMF: $OVMF_CODE"
echo "[uefi] Medio: ${MEDIA[*]}  (timeout ${TIMEOUT_SEC}s)"
: > "$LOG"

# La entrada serie de QEMU (stdin) viene de un FIFO: así podemos "teclear"
# en el shell de NimbusOS cuando aparezca el prompt.
qemu-system-x86_64 \
  -machine q35 -m 256 \
  -drive "if=pflash,format=raw,readonly=on,file=${OVMF_CODE}" \
  -drive "if=pflash,format=raw,file=${VARS}" \
  "${MEDIA[@]}" \
  -net none -serial stdio -display none -no-reboot \
  < "$FIFO" > "$LOG" 2>&1 &
QEMU_PID=$!
exec 3> "$FIFO"          # mantener el FIFO abierto para escritura

cleanup() {
  exec 3>&- 2>/dev/null || true
  kill -KILL "$QEMU_PID" 2>/dev/null || true
  wait "$QEMU_PID" 2>/dev/null || true
  rm -rf "$WORK"
}
trap cleanup EXIT

# Texto limpio (sin secuencias ANSI de la consola de OVMF)
clean_log() { sed 's/\x1b\[[0-9;?=]*[A-Za-z]//g' "$LOG" | tr -d '\r'; }

prompt=0 ; ok=0
for ((i=0; i<TIMEOUT_SEC*10; i++)); do
  if [[ $prompt -eq 0 ]] && clean_log | grep -q "NimbusOS shell lista"; then
    prompt=1
    sleep 0.3
    printf 'about\r' >&3      # comando real al shell serie
  fi
  if [[ $prompt -eq 1 ]] && clean_log | grep -q "Boot: UEFI (BOOTX64.EFI"; then
    ok=1; sleep 0.2; break
  fi
  kill -0 "$QEMU_PID" 2>/dev/null || break
  sleep 0.1
done

cleanup ; trap - EXIT

echo "----- serial (UEFI, desde BOOTX64.EFI) -----"
clean_log | sed -n '/NimbusOS UEFI bootmgr/,$p' | grep -v '^\s*$'
echo "--------------------------------------------"
echo "[uefi] log completo: $LOG"

CL="$(clean_log)"
for needle in "NimbusOS UEFI bootmgr" "ExitBootServices OK" "Boot: UEFI OK" \
              "NimbusOS: kernel freestanding listo" "NimbusOS shell lista"; do
  if ! grep -qF "$needle" <<<"$CL"; then
    echo "[uefi] FALLO — falta: '$needle'" >&2; exit 1
  fi
done
if [[ $ok -ne 1 ]]; then
  echo "[uefi] FALLO — el shell no respondió a 'about'" >&2; exit 1
fi
echo "[uefi] OK — NimbusOS arrancó por UEFI/OVMF (${MODE}) y el shell respondió"
exit 0
