#include "pmm.h"
#include "../drivers/vga.h"

static uint32_t pmm_bitmap[PMM_MAX_MEMORY / PMM_BLOCK_SIZE / PMM_BLOCKS_PER_BYTE];
static pmm_state_t pmm_state;

static inline void bitmap_set(uint32_t bit) {
    pmm_state.bitmap[bit / 32] |= (1 << (bit % 32));
}

static inline void bitmap_clear(uint32_t bit) {
    pmm_state.bitmap[bit / 32] &= ~(1 << (bit % 32));
}

static inline bool bitmap_test(uint32_t bit) {
    return pmm_state.bitmap[bit / 32] & (1 << (bit % 32));
}

static int32_t pmm_find_free_block() {
    for (uint32_t i = 0; i < pmm_state.total_blocks; i++) {
        if (!bitmap_test(i)) return (int32_t)i;
    }
    return -1;
}

static int32_t pmm_find_free_blocks(size_t count) {
    uint32_t start = 0;
    uint32_t found = 0;

    for (uint32_t i = 0; i < pmm_state.total_blocks; i++) {
        if (!bitmap_test(i)) {
            if (found == 0) start = i;
            found++;
            if (found == count) return (int32_t)start;
        } else {
            found = 0;
        }
    }
    return -1;
}

void pmm_init(uint32_t mem_size) {
    pmm_state.memory_size   = mem_size;
    pmm_state.total_blocks  = mem_size / PMM_BLOCK_SIZE;
    pmm_state.used_blocks   = pmm_state.total_blocks;
    pmm_state.free_blocks   = 0;
    pmm_state.bitmap        = pmm_bitmap;

    for (uint32_t i = 0; i < pmm_state.total_blocks / 32; i++) {
        pmm_state.bitmap[i] = 0xFFFFFFFF;
    }

    uint32_t kernel_end = 0x100000;
    uint32_t usable_start = kernel_end / PMM_BLOCK_SIZE;

    for (uint32_t i = usable_start; i < pmm_state.total_blocks; i++) {
        bitmap_clear(i);
        pmm_state.used_blocks--;
        pmm_state.free_blocks++;
    }

    vga_printf("[PMM]  Total: %d KB | Free: %d KB | Used: %d KB\n",
        (pmm_state.total_blocks * PMM_BLOCK_SIZE) / 1024,
        (pmm_state.free_blocks  * PMM_BLOCK_SIZE) / 1024,
        (pmm_state.used_blocks  * PMM_BLOCK_SIZE) / 1024
    );
}

void *pmm_alloc_block() {
    if (pmm_state.free_blocks == 0) return NULL;

    int32_t block = pmm_find_free_block();
    if (block == -1) return NULL;

    bitmap_set((uint32_t)block);
    pmm_state.used_blocks++;
    pmm_state.free_blocks--;

    return (void*)(uint32_t)(block * PMM_BLOCK_SIZE);
}

void *pmm_alloc_blocks(size_t count) {
    if (pmm_state.free_blocks < count) return NULL;

    int32_t start = pmm_find_free_blocks(count);
    if (start == -1) return NULL;

    for (size_t i = 0; i < count; i++) {
        bitmap_set((uint32_t)(start + i));
    }

    pmm_state.used_blocks += count;
    pmm_state.free_blocks -= count;

    return (void*)(uint32_t)(start * PMM_BLOCK_SIZE);
}

void pmm_free_block(void *ptr) {
    uint32_t block = (uint32_t)ptr / PMM_BLOCK_SIZE;
    bitmap_clear(block);
    pmm_state.used_blocks--;
    pmm_state.free_blocks++;
}

void pmm_free_blocks(void *ptr, size_t count) {
    uint32_t block = (uint32_t)ptr / PMM_BLOCK_SIZE;
    for (size_t i = 0; i < count; i++) {
        bitmap_clear(block + i);
    }
    pmm_state.used_blocks -= count;
    pmm_state.free_blocks += count;
}

uint32_t pmm_get_free_blocks()  { return pmm_state.free_blocks; }
uint32_t pmm_get_used_blocks()  { return pmm_state.used_blocks; }
uint32_t pmm_get_total_blocks() { return pmm_state.total_blocks; }

bool pmm_is_block_free(uint32_t block) {
    return !bitmap_test(block);
}

void pmm_dump_stats() {
    vga_printf("[PMM]  Blocks => Total: %d | Used: %d | Free: %d\n",
        pmm_state.total_blocks,
        pmm_state.used_blocks,
        pmm_state.free_blocks
    );
}
