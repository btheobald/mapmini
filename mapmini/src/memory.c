#include "memory.h"
#include <stdio.h>

volatile uint8_t reg[ARENA_DEFAULT_SIZE];

void arena_init(arena_t * arena, size_t size) {
    arena->region = (uint8_t *)&reg;
    arena->size = sizeof(uint8_t)*size;
    arena->current = 0;
}

void* arena_malloc(arena_t * arena, size_t size) {
    if(arena->current+size > arena->size) {
        return NULL;
    } 
    
    size_t tmp = arena->region+arena->current;
    arena->current += size;

    return (void *) (tmp);
}

size_t arena_free(arena_t * arena) { // Invalidates existing pointers
    size_t oldsize = arena->current;
    arena->current = 0;
    //free(arena->region);
    return oldsize;
}