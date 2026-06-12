#ifndef PMM_H
#define PMM_H

#include "../lib/types.h"

#define PMM_BLOCK_SIZE      4096
#define PMM_BLOCKS_PER_BYTE 8
#define PMM_MAX_MEMORY      0x10000000

#define PMM_BLOCK_FREE      0
#define PMM_BLOCK_USED      1

typedef struct {
    uint32_t total_blocks;
    uint32_t used_blocks;
    uint32_t free_blocks;
    uint32_t memory_size;
    uint32_t *bitmap;
} pmm_state_t;

void     pmm_init(uint32_t mem_size);
void    *pmm_alloc_block();
void    *pmm_alloc_blocks(size_t count);
void     pmm_free_block(void *ptr);
void     pmm_free_blocks(void *ptr, size_t count);
uint32_t pmm_get_free_blocks();
uint32_t pmm_get_used_blocks();
uint32_t pmm_get_total_blocks();
bool     pmm_is_block_free(uint32_t block);
void     pmm_dump_stats();

#endif
