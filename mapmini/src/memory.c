#include "memory.h"
#include <stdio.h>

uint8_t reg[ARENA_DEFAULT_SIZE];

void arena_init(arena_t * arena, uint64_t size) {
    arena->region = (uint8_t*)&reg;
    arena->size = sizeof(uint8_t)*size;
    arena->current = 0;
}

void* arena_malloc(arena_t * arena, uint64_t size) {
    if(arena->current+size > arena->size) return NULL;
    arena->current += size;
    printf("%016llX\n", arena->region+arena->current);
    return (void *) (arena->region+arena->current);
}

uint64_t arena_free(arena_t * arena) { // Invalidates existing pointers
    uint64_t oldsize = arena->current;
    arena->current = 0;
    //free(arena->region);
    return oldsize;
}