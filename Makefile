CC      = i686-elf-gcc
AS      = nasm
LD      = i686-elf-ld
QEMU    = qemu-system-i386

CFLAGS  = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
          -nostartfiles -nodefaultlibs -Wall -Wextra -Werror \
          -ffreestanding -O2 -std=c99

ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -T scripts/linker.ld

BUILD   = build
ISO_DIR = build/iso

C_SRCS  = kernel/kernel.c \
          kernel/pmm.c \
          kernel/vmm.c \
          kernel/ipc.c \
          kernel/scheduler.c \
          drivers/vga.c

ASM_SRCS = kernel/kernel_entry.asm

C_OBJS   = $(patsubst %.c,   $(BUILD)/%.o, $(C_SRCS))
ASM_OBJS = $(patsubst %.asm, $(BUILD)/%.o, $(ASM_SRCS))

OBJS = $(ASM_OBJS) $(C_OBJS)

.PHONY: all clean run iso

all: $(BUILD)/voidos.bin

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: %.asm
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD)/voidos.bin: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

$(BUILD)/boot.bin: boot/bootloader.asm
	@mkdir -p $(BUILD)
	$(AS) -f bin boot/bootloader.asm -o $@

iso: $(BUILD)/voidos.bin
	@mkdir -p $(ISO_DIR)/boot/grub
	cp $(BUILD)/voidos.bin $(ISO_DIR)/boot/voidos.bin
	cp scripts/grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o $(BUILD)/voidos.iso $(ISO_DIR)

run: $(BUILD)/voidos.bin
	$(QEMU) -kernel $(BUILD)/voidos.bin -m 32M

run-iso: iso
	$(QEMU) -cdrom $(BUILD)/voidos.iso -m 32M

clean:
	rm -rf $(BUILD)

debug: $(BUILD)/voidos.bin
	$(QEMU) -kernel $(BUILD)/voidos.bin -m 32M -s -S &
	gdb $(BUILD)/voidos.bin \
		-ex "target remote localhost:1234" \
		-ex "symbol-file $(BUILD)/voidos.bin" \
		-ex "break kernel_main" \
		-ex "continue"
