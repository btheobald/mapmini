#ifndef MEM_GUARD
#define MEM_GUARD

#include <stdlib.h>
#include <stdint.h>

#define ARENA_DEFAULT_SIZE 100000

typedef struct {
    uint8_t *region;
    uint64_t size;
    uint64_t current;
} arena_t;

void  arena_init(arena_t * arena, uint64_t size);
void* arena_malloc(arena_t * arena, uint64_t size);
uint64_t arena_free(arena_t * arena);

#endif
