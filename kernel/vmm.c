#include "vmm.h"
#include "pmm.h"
#include "../drivers/vga.h"

static page_directory_t *kernel_dir = NULL;
static page_directory_t *current_dir = NULL;

static inline pte_t *vmm_get_pte(page_table_t *table, uintptr_t virt) {
    return &table->entries[(virt >> 12) & 0x3FF];
}

static inline pde_t *vmm_get_pde(page_directory_t *dir, uintptr_t virt) {
    return &dir->entries[virt >> 22];
}

static inline void vmm_flush_tlb(uintptr_t virt) {
    __asm__ volatile("invlpg (%0)" :: "r"(virt) : "memory");
}

static inline void vmm_load_cr3(uint32_t addr) {
    __asm__ volatile("mov %0, %%cr3" :: "r"(addr) : "memory");
}

static inline void vmm_enable_paging() {
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    __asm__ volatile("mov %0, %%cr0" :: "r"(cr0) : "memory");
}

void vmm_map_page(page_directory_t *dir, uintptr_t virt, uintptr_t phys, uint32_t flags) {
    pde_t *pde = vmm_get_pde(dir, virt);

    if (!(*pde & VMM_PAGE_PRESENT)) {
        page_table_t *table = (page_table_t*)pmm_alloc_block();
        if (!table) return;

        for (int i = 0; i < VMM_PAGES_PER_TABLE; i++) {
            table->entries[i] = 0;
        }

        *pde = ((uint32_t)table & 0xFFFFF000) | VMM_PAGE_PRESENT | VMM_PAGE_WRITABLE;
    }

    page_table_t *table = (page_table_t*)(*pde & 0xFFFFF000);
    pte_t *pte = vmm_get_pte(table, virt);
    *pte = (phys & 0xFFFFF000) | (flags & 0xFFF) | VMM_PAGE_PRESENT;

    vmm_flush_tlb(virt);
}

void vmm_unmap_page(page_directory_t *dir, uintptr_t virt) {
    pde_t *pde = vmm_get_pde(dir, virt);
    if (!(*pde & VMM_PAGE_PRESENT)) return;

    page_table_t *table = (page_table_t*)(*pde & 0xFFFFF000);
    pte_t *pte = vmm_get_pte(table, virt);
    *pte = 0;

    vmm_flush_tlb(virt);
}

bool vmm_is_mapped(page_directory_t *dir, uintptr_t virt) {
    pde_t *pde = vmm_get_pde(dir, virt);
    if (!(*pde & VMM_PAGE_PRESENT)) return false;

    page_table_t *table = (page_table_t*)(*pde & 0xFFFFF000);
    pte_t *pte = vmm_get_pte(table, virt);
    return (*pte & VMM_PAGE_PRESENT) != 0;
}

void vmm_switch_directory(page_directory_t *dir) {
    current_dir = dir;
    vmm_load_cr3((uint32_t)dir->entries);
}

page_directory_t *vmm_get_kernel_dir() {
    return kernel_dir;
}

page_directory_t *vmm_create_directory() {
    page_directory_t *dir = (page_directory_t*)pmm_alloc_block();
    if (!dir) return NULL;

    for (int i = 0; i < VMM_TABLES_PER_DIR; i++) {
        dir->entries[i] = 0;
    }

    for (int i = 0; i < VMM_TABLES_PER_DIR / 4; i++) {
        dir->entries[i] = kernel_dir->entries[i];
    }

    return dir;
}

void vmm_destroy_directory(page_directory_t *dir) {
    if (!dir || dir == kernel_dir) return;

    for (int i = 0; i < VMM_TABLES_PER_DIR; i++) {
        if (dir->entries[i] & VMM_PAGE_PRESENT) {
            page_table_t *table = (page_table_t*)(dir->entries[i] & 0xFFFFF000);
            pmm_free_block(table);
        }
    }

    pmm_free_block(dir);
}

void vmm_init() {
    kernel_dir = (page_directory_t*)pmm_alloc_block();
    if (!kernel_dir) return;

    for (int i = 0; i < VMM_TABLES_PER_DIR; i++) {
        kernel_dir->entries[i] = 0;
    }

    uintptr_t phys = 0x0;
    uintptr_t virt = VMM_KERNEL_BASE;

    while (phys < 0x400000) {
        vmm_map_page(kernel_dir, virt, phys, VMM_PAGE_WRITABLE);
        phys += VMM_PAGE_SIZE;
        virt += VMM_PAGE_SIZE;
    }

    vmm_switch_directory(kernel_dir);
    vmm_enable_paging();

    vga_puts("[VMM]  Paging enabled | Kernel mapped at 0x00000000\n");
}
