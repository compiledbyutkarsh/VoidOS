# VoidOS

A minimal x86 microkernel operating system written in C and x86 Assembly.

## Architecture

VoidOS follows a microkernel design — only the bare essentials run in kernel space. Everything else lives in userspace, communicating via a clean IPC interface.
+------------------+
|   User Processes |
+------------------+
|   IPC / Ports    |
+------------------+
|   Scheduler      |
+------------------+
|   VMM  |  PMM   |
+------------------+
|   Microkernel    |
+------------------+
|   Hardware (x86) |
+------------------+
## Features

- Custom two-stage bootloader in x86 Assembly
- Microkernel architecture with minimal kernel footprint
- Physical memory manager using a bitmap allocator
- Virtual memory manager with paging and TLB management
- Priority-based preemptive task scheduler
- Message-passing IPC via port abstraction
- VGA text mode driver with color support and printf

## Build Requirements

- i686-elf-gcc — cross compiler
- nasm — assembler
- qemu-system-i386 — emulator
- grub-mkrescue — bootable ISO generation
- xorriso — ISO utility

## Building

    make all

## Running

    make run

## Bootable ISO

    make iso
    make run-iso

## Debug

    make debug

## Project Structure

    VoidOS/
    boot/           Bootloader (Assembly)
    kernel/         Kernel core (C + ASM)
    drivers/        Hardware drivers
    lib/            Base types and utilities
    userspace/      User programs
    scripts/        Linker script, GRUB config
    tools/          Build utilities

## License

MIT
