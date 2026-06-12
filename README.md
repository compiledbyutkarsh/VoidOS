# 🖤 VoidOS

> A minimal x86 microkernel operating system built from scratch in C and x86 Assembly.

![Language](https://img.shields.io/badge/Language-C%20%2B%20Assembly-blue?style=flat-square)
![Architecture](https://img.shields.io/badge/Architecture-x86-red?style=flat-square)
![Kernel](https://img.shields.io/badge/Kernel-Microkernel-purple?style=flat-square)
![Status](https://img.shields.io/badge/Status-Booting-brightgreen?style=flat-square)

---

## 🧠 What is VoidOS?

VoidOS is a bare-metal x86 operating system written entirely from scratch — no Linux, no POSIX, no borrowed kernel code. Just raw C and Assembly running directly on hardware. It boots, manages memory, schedules tasks, and passes messages between processes — all without a single line of standard library.

This is what systems programming looks like at its lowest level.

---

## ⚙️ Architecture

VoidOS follows a **microkernel design** — only the absolute minimum runs in kernel space. Everything else communicates through a clean message-passing IPC interface.

┌─────────────────────────────┐
│        User Processes        │
├─────────────────────────────┤
│       IPC Message Ports      │
├─────────────────────────────┤
│     Priority Scheduler       │
├──────────────┬──────────────┤
│     VMM      │     PMM      │
├─────────────────────────────┤
│        Microkernel           │
├─────────────────────────────┤
│      Hardware  (x86)         │
└─────────────────────────────┘

---

## 🚀 Features

### 🔧 Bootloader
- Two-stage bootloader written in pure x86 Assembly
- Switches CPU from 16-bit Real Mode → 32-bit Protected Mode
- Sets up GDT (Global Descriptor Table) before jumping to kernel

### 🧩 Microkernel
- Minimal kernel footprint — only core services in kernel space
- Multiboot compliant — boots via GRUB or QEMU directly
- Full C + Assembly hybrid core

### 🗂️ Physical Memory Manager (PMM)
- Bitmap-based block allocator
- 4 KB block granularity
- Single and multi-block allocation support
- Real-time free/used block tracking

### 📄 Virtual Memory Manager (VMM)
- x86 two-level paging (Page Directory + Page Tables)
- TLB flush on page map/unmap
- Kernel identity mapping at boot
- Per-process page directory support

### 📬 IPC — Inter-Process Communication
- Port-based message passing
- 256 concurrent ports
- 64-message circular queue per port
- Timestamped messages via RDTSC

### ⚡ Scheduler
- Priority-based preemptive scheduling
- 5 priority levels: IDLE → LOW → NORMAL → HIGH → REALTIME
- Task states: RUNNING, READY, BLOCKED, SLEEPING, ZOMBIE
- Per-task IPC port auto-assigned on creation

### 🖥️ VGA Driver
- Direct VGA text mode memory access (0xB8000)
- 16-color foreground and background support
- Hardware cursor control via I/O ports
- Built-in printf with %d, %x, %s, %c format support
- Auto-scroll on overflow

---

## 📁 Project Structure

VoidOS/
├── boot/
│   ├── bootloader.asm      # Stage 1 bootloader
│   ├── print.asm           # Real mode print utilities
│   ├── disk.asm            # BIOS disk read routines
│   ├── gdt.asm             # Global Descriptor Table
│   └── pm_switch.asm       # Protected mode switch
├── kernel/
│   ├── kernel_entry.asm    # Kernel entry point (ASM)
│   ├── multiboot.asm       # Multiboot header
│   ├── kernel.c            # Kernel main
│   ├── pmm.c / pmm.h       # Physical memory manager
│   ├── vmm.c / vmm.h       # Virtual memory manager
│   ├── ipc.c / ipc.h       # IPC subsystem
│   └── scheduler.c / .h    # Task scheduler
├── drivers/
│   └── vga.c / vga.h       # VGA text mode driver
├── lib/
│   └── types.h             # Base type definitions
├── scripts/
│   ├── linker.ld           # Linker script
│   └── grub.cfg            # GRUB boot config
└── Makefile

---

## 🛠️ Build Requirements

| Tool | Purpose |
|------|---------|
| `i686-elf-gcc` | Cross compiler for x86 freestanding |
| `nasm` | x86 assembler |
| `qemu-system-i386` | x86 emulator for testing |
| `grub-mkrescue` | Bootable ISO generation |
| `xorriso` | ISO creation utility |

### Install on macOS

```bash
brew install i686-elf-gcc nasm qemu xorriso grub
```

---

## 🔨 Building & Running

```bash
# Build the kernel
make all

# Run in QEMU
make run

# Build bootable ISO
make iso

# Run ISO in QEMU
make run-iso

# Debug with GDB
make debug

# Clean build artifacts
make clean
```

---

## 📸 Boot Output

=================================================
VoidOS Microkernel v0.1.0
x86 Architecture | C + ASM Core
[BOOT]  VoidOS kernel loaded
[MEM]   Initializing physical memory manager...
[PMM]   Total: 16384 KB | Free: 15360 KB | Used: 1024 KB
[MEM]   Initializing virtual memory manager...
[VMM]   Paging enabled | Kernel mapped at 0x00000000
[IPC]   Initializing IPC subsystem...
[IPC]   Port table initialized | Max ports: 256
[SCHED] Initializing scheduler...
[SCHED] Task created | PID: 1 | Name: idle | Priority: 0
[SCHED] Initialized | Max tasks: 64 | Quantum: 10
[BOOT]  All subsystems nominal
[BOOT]  VoidOS is alive.

---

## 📌 Roadmap

- [ ] Keyboard driver (PS/2)
- [ ] Basic filesystem (VoidFS)
- [ ] Userspace process loader
- [ ] System call interface
- [ ] Shell (VoidShell)
- [ ] ELF binary loader

---

<p align="center">Made with 🖤 by <a href="https://github.com/compiledbyutkarsh">compiled by utkarsh</a></p>
