#ifndef MEM_GUARD
#define MEM_GUARD

#include <stdlib.h>
#include <stdint.h>

void init_arena(uint16_t size);
void reset_arena();
void free_arena();

uint32_t* mark_bytes(uint16_t size);

#endif
