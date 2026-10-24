# NimbusOS — BOOTX64.EFI con gnu-efi (apt: gnu-efi)
# Uso: make -C boot/uefi/app OUTDIR=/ruta/out
ROOT    := $(abspath ../../..)
OUTDIR  ?= $(ROOT)/out
OBJDIR  := $(OUTDIR)/uefi
EFI     := $(OUTDIR)/BOOTX64.EFI

EFIINC  ?= /usr/include/efi
EFILIB  ?= /usr/lib
LDS     := $(EFILIB)/elf_x86_64_efi.lds
CRT0    := $(EFILIB)/crt0-efi-x86_64.o

CFLAGS := -I$(EFIINC) -I$(EFIINC)/x86_64 -I$(EFIINC)/protocol \
          -fpic -ffreestanding -fno-stack-protector -fno-stack-check \
          -fshort-wchar -mno-red-zone -maccumulate-outgoing-args \
          -Wall -Wextra -Wno-unused-parameter -O2
LDFLAGS := -nostdlib -znocombreloc -shared -Bsymbolic -T $(LDS) -L$(EFILIB)

.PHONY: all clean
all: $(EFI)

$(OBJDIR):
	mkdir -p $@

$(OBJDIR)/bootx64.o: bootx64.c ../../freestanding/nimbus_bootinfo.h | $(OBJDIR)
	gcc $(CFLAGS) -c $< -o $@

$(OBJDIR)/bootx64.so: $(OBJDIR)/bootx64.o
	ld $(LDFLAGS) $(CRT0) $< -o $@ -lgnuefi -lefi

# Conversión ELF (shared) → PE32+ subsistema 10 (EFI application)
$(EFI): $(OBJDIR)/bootx64.so
	objcopy -j .text -j .sdata -j .data -j .rodata -j .dynamic -j .dynsym \
	        -j .rel -j .rela -j '.rel.*' -j '.rela.*' -j .reloc \
	        --target efi-app-x86_64 --subsystem=10 $< $@
	@echo "NimbusOS UEFI app: $@"

clean:
	rm -rf $(OBJDIR) $(EFI)
