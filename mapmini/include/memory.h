#ifndef MEM_GUARD
#define MEM_GUARD

#include <stdlib.h>
#include <stdint.h>

#define ARENA_DEFAULT_SIZE 128000

typedef struct {
    uint8_t *region;
    size_t size;
    size_t current;
} arena_t;

void  arena_init(arena_t * arena, size_t size);
void* arena_malloc(arena_t * arena, size_t size);
size_t arena_free(arena_t * arena);

#endif
