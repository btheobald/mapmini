#ifndef FILE_GUARD
#define FILE_GUARD

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "io_posix.h"

uint8_t get_uint8(fb_handler * fbh);
uint16_t get_uint16(fb_handler * fbh);
uint32_t get_uint32(fb_handler * fbh);
uint64_t get_uint64(fb_handler * fbh);

int_least8_t get_int8(fb_handler * fbh);
int16_t get_int16(fb_handler * fbh);
int32_t get_int32(fb_handler * fbh);
int64_t get_int64(fb_handler * fbh);

uint64_t get_varint(fb_handler * fbh, uint8_t len);
uint32_t get_vbe_uint(fb_handler * fbh);
int32_t get_vbe_int(fb_handler * fbh);

void get_string(fb_handler * fbh, char * ptr, uint8_t len);

#endif
