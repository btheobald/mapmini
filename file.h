#ifndef FILE_GUARD
#define FILE_GUARD

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "ff.h"

#define FILE_READ_BUFFER_SIZE 10240  

typedef struct _file_buffer_tracker {
    FIL * fp;
    uint8_t * buffer_ptr;
    uint16_t buffer_pos;
    uint16_t bytes_read;
} fb_tracker;

uint8_t init_buffer(FIL * fp, fb_tracker * fbt, uint8_t * buffer);

uint16_t load_buffer(fb_tracker * fbt);

void get_cross_buffer(fb_tracker * fbt, uint8_t * tmp, uint8_t bytes_req);

void relative_reset_buffer(fb_tracker * fbt, uint16_t seek);

uint8_t get_uint8(fb_tracker * fbt);

uint16_t get_uint16(fb_tracker * fbt);

uint32_t get_uint32(fb_tracker * fbt);

uint64_t get_uint64(fb_tracker * fbt);

int_least8_t get_int8(fb_tracker * fbt);

int16_t get_int16(fb_tracker * fbt);

int32_t get_int32(fb_tracker * fbt);

int64_t get_int64(fb_tracker * fbt);

uint64_t get_varint(fb_tracker * fbt, uint8_t len);

void get_string(fb_tracker * fbt, char * ptr, uint8_t len);

uint32_t get_vbe_uint(fb_tracker * fbt);

int32_t get_vbe_int(fb_tracker * fbt);

#endif
