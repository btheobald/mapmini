#include "memory.h"

uint8_t reg[ARENA_DEFAULT_SIZE];

void arena_init(arena_t * arena, uint32_t size) {
    arena->region = (uint8_t*)&reg;//(malloc(sizeof(uint8_t)*size));
    arena->size = sizeof(uint8_t)*size;
    arena->current = 0;
}

void* arena_malloc(arena_t * arena, size_t size) {
    if(arena->current+size > arena->size) return NULL;
    arena->current += size;
    return (void *) (arena->region+arena->current);
}

size_t arena_free(arena_t * arena) { // Invalidates existing pointers
    size_t oldsize = arena->current;
    arena->current = 0;
    //free(arena->region);
    return oldsize;
}