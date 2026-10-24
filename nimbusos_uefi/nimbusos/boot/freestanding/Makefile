# NimbusOS — build del kernel freestanding (Multiboot2 ELF64)
# Uso: make -C boot/freestanding OUTDIR=../../out

ROOT     := $(abspath ../..)
OUTDIR   ?= $(ROOT)/out
OBJDIR   := $(OUTDIR)/freestanding
KERNEL   := $(OUTDIR)/kernel.elf

CC  := gcc
AS  := nasm
LD  := ld

CFLAGS := -ffreestanding -fno-stack-protector -fno-pic -fno-pie \
          -mno-red-zone -m64 -Wall -Wextra -O2 -std=c11 \
          -fno-asynchronous-unwind-tables -nostdlib
ASFLAGS := -f elf64
LDFLAGS := -nostdlib -static -z max-page-size=0x1000 -T linker.ld

C_SRCS := serial.c string.c kprint.c mm_early.c shell.c kmain.c
OBJS   := $(OBJDIR)/boot.o $(addprefix $(OBJDIR)/,$(C_SRCS:.c=.o))

.PHONY: all clean

all: $(KERNEL)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/boot.o: boot.asm | $(OBJDIR)
	$(AS) $(ASFLAGS) $< -o $@

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CFLAGS) -I. -c $< -o $@

$(KERNEL): $(OBJS) linker.ld
	mkdir -p $(OUTDIR)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
	@echo "NimbusOS kernel: $@"
	@command -v file >/dev/null && file $@ || ls -la $@

clean:
	rm -rf $(OBJDIR) $(KERNEL)
