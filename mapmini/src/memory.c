#include "memory.h"
#include <stdio.h>

uint32_t current_pos;
uint32_t * arena;
uint32_t arena_size;

uint32_t * init_arena(uint32_t size) {
    current_pos = 0;
    arena = malloc(size);
    arena_size = size;
    return arena;
}

void reset_arena() {
    current_pos = 0;
}

void free_arena() {
    printf("Arena %d/%d\n\r", current_pos, arena_size);
    free(arena);
}

void* mark_bytes(uint16_t size) {
    current_pos += size;
    printf("Arena %d/%d\n\r", current_pos, arena_size);
    return malloc(size); //(void *)(arena+current_pos);
}
