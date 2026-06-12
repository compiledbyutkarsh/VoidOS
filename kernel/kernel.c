#include "../lib/types.h"
#include "../drivers/vga.h"
#include "../kernel/ipc.h"
#include "../kernel/scheduler.h"
#include "../kernel/pmm.h"
#include "../kernel/vmm.h"

static void print_boot_banner() {
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("=================================================\n");
    vga_puts("          VoidOS Microkernel v0.1.0              \n");
    vga_puts("       x86 Architecture | C + ASM Core           \n");
    vga_puts("=================================================\n");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
}

void kernel_main(uint32_t magic, uint32_t addr) {
    UNUSED(magic);
    UNUSED(addr);
    vga_init();
    print_boot_banner();

    vga_puts("[BOOT] VoidOS kernel loaded\n");

    vga_puts("[MEM]  Initializing physical memory manager...\n");
    pmm_init(0x1000000);

    vga_puts("[MEM]  Initializing virtual memory manager...\n");
    vmm_init();

    vga_puts("[IPC]  Initializing IPC subsystem...\n");
    ipc_init();

    vga_puts("[SCHED] Initializing scheduler...\n");
    scheduler_init();

    vga_puts("[BOOT] All subsystems nominal\n");
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts("[BOOT] VoidOS is alive.\n");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);

    for (;;) {
        __asm__ volatile("hlt");
    }
}
