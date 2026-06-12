#ifndef VMM_H
#define VMM_H

#include "../lib/types.h"

#define VMM_PAGE_SIZE       4096
#define VMM_PAGE_PRESENT    (1 << 0)
#define VMM_PAGE_WRITABLE   (1 << 1)
#define VMM_PAGE_USER       (1 << 2)
#define VMM_PAGE_ACCESSED   (1 << 5)
#define VMM_PAGE_DIRTY      (1 << 6)

#define VMM_PAGES_PER_TABLE 1024
#define VMM_TABLES_PER_DIR  1024

#define VMM_KERNEL_BASE     0x00000000
#define VMM_USER_BASE       0x40000000
#define VMM_USER_STACK      0xBFFFF000

typedef uint32_t pte_t;
typedef uint32_t pde_t;

typedef struct {
    pte_t entries[VMM_PAGES_PER_TABLE];
} page_table_t;

typedef struct {
    pde_t entries[VMM_TABLES_PER_DIR];
} page_directory_t;

void  vmm_init();
void  vmm_map_page(page_directory_t *dir, uintptr_t virt, uintptr_t phys, uint32_t flags);
void  vmm_unmap_page(page_directory_t *dir, uintptr_t virt);
bool  vmm_is_mapped(page_directory_t *dir, uintptr_t virt);
void  vmm_switch_directory(page_directory_t *dir);
page_directory_t *vmm_get_kernel_dir();
page_directory_t *vmm_create_directory();
void  vmm_destroy_directory(page_directory_t *dir);

#endif
