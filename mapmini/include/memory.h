#ifndef MEM_GUARD
#define MEM_GUARD

#include <stdlib.h>
#include <stdint.h>

#define ARENA_SIZE 100000

uint32_t * init_arena(uint32_t size);
void reset_arena();
void free_arena();

void* mark_bytes(uint16_t size);

#endif
