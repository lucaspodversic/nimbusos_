NimbusOS — paquete de arranque UEFI + BIOS (código fuente + imágenes)
====================================================================
Marca: NimbusOS (clean-room). No es Windows.

Imágenes incluidas (nimbusos/out/):
  nimbusos_uefi.img  disco GPT 64 MiB con ESP FAT32 (/EFI/BOOT/BOOTX64.EFI, /NIMBUS/KERNEL.ELF)
  nimbusos_uefi.iso  ISO UEFI (El Torito EFI)
  nimbusos.iso       ISO BIOS (Multiboot2 + GRUB)
  BOOTX64.EFI, kernel.elf, logs serie de QEMU

Reconstruir y probar:   ./REBUILD.sh          (o ./REBUILD.sh --no-test)
Solo UEFI:              cd nimbusos && python3 build/make_uefi.py && ./tests/boot/test_qemu_uefi.sh

Arrancar a mano con OVMF:
  cp /usr/share/OVMF/OVMF_VARS_4M.fd /tmp/vars.fd
  qemu-system-x86_64 -machine q35 -m 256 \
    -drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_CODE_4M.fd \
    -drive if=pflash,format=raw,file=/tmp/vars.fd \
    -drive format=raw,file=nimbusos/out/nimbusos_uefi.img -net none -serial stdio -display none

Código: nimbusos/boot/uefi/app/bootx64.c (cargador), nimbusos/boot/freestanding/ (kernel)
