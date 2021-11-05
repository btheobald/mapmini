#include "mem.h"
#include <stdio.h>

#define ARENA_SIZE 100000
static uint32_t current_pos;
static uint8_t arena[ARENA_SIZE];

void init_arena(uint16_t size) {
    current_pos = 0;
    //arena_size = size;
    //arena = malloc(size);
}

void reset_arena() {
    current_pos = 0;
}

void free_arena() {
    printf("Arena %d/%d\n\r", current_pos, ARENA_SIZE);
    //free(arena);
}

uint32_t* mark_bytes(uint16_t size) {
    current_pos += size;
    return (void *)(arena+current_pos);
}
