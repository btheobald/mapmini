#include "file.h"

uint8_t init_buffer(FILE * fp, fb_tracker * fbt, uint8_t * buffer) {
    fbt->buffer_pos = 0;
    fbt->buffer_ptr = buffer;
    fbt->bytes_read = 0;
    fbt->fp = fp;

    load_buffer(fbt); 
}

// Returns bytes read from file
uint16_t load_buffer(fb_tracker * fbt) {

    //printf("\033[0;31ml\033[0m");

    fbt->buffer_pos = 0; // Reset buffer offset
    //f_read(fbt->fp, fbt->buffer_ptr, FILE_READ_BUFFER_SIZE, &(fbt->bytes_read));
    return fbt->bytes_read;
}

uint16_t get_remaining_bytes(fb_tracker * fbt) {
    return (fbt->bytes_read - fbt->buffer_pos);
}

// Get integer when it lies on a buffer boundary, load bytewise for simplicity
// TODO: Profile performance of this function
void get_cross_buffer(fb_tracker * fbt, uint8_t * tmp, uint8_t bytes_req) {
    // Pre-load copy
    int pre;
    for(pre = 0; pre < get_remaining_bytes(fbt); pre++) {
        tmp[pre] = *((uint8_t*)(fbt->buffer_ptr + fbt->buffer_pos));
        fbt->buffer_ptr++;
    }

    load_buffer(fbt);

    // Post-load copy
    for(int post = pre; post < bytes_req; post++) {
        tmp[post] = *((uint8_t*)(fbt->buffer_ptr + fbt->buffer_pos));
        fbt->buffer_ptr++;
    }
}

void relative_reset_buffer(fb_tracker * fbt, uint16_t seek) {
    //printf("Rel Reset - %d\n\r", seek);
    fbt->buffer_pos = 0;
    //fseek(fbt->fp, -seek, SEEK_CUR);
    //f_lseek(fbt->fp, f_tell(fbt->fp)-seek);
    load_buffer(fbt);
}

uint8_t get_uint8(fb_tracker * fbt) {
    // Don't overflow the buffer, reload it.
    if(!get_remaining_bytes(fbt)) {
        load_buffer(fbt);
    } 
    
    uint8_t byte = *((uint8_t*)(fbt->buffer_ptr + fbt->buffer_pos));
    fbt->buffer_pos++;
    return byte;
}

uint16_t get_uint16(fb_tracker * fbt) {
    uint8_t len = 2;
    uint16_t val;

    // Check Highwater, reload buffer if needed
    if(fbt->buffer_pos + len > FILE_READ_BUFFER_SIZE) {
        relative_reset_buffer(fbt, FILE_READ_BUFFER_SIZE - fbt->buffer_pos);
    }

    val = *((uint16_t*)(fbt->buffer_ptr + fbt->buffer_pos));
    fbt->buffer_pos += len;
    return __builtin_bswap16(val);
}

uint32_t get_uint32(fb_tracker * fbt) {
    uint8_t len = 4;
    uint32_t val;

        // Check Highwater, reload buffer if needed
    if(fbt->buffer_pos + len > FILE_READ_BUFFER_SIZE) {
        relative_reset_buffer(fbt, FILE_READ_BUFFER_SIZE - fbt->buffer_pos);
    }

    val = *((uint32_t*)(fbt->buffer_ptr + fbt->buffer_pos));
    fbt->buffer_pos += len;

    return __builtin_bswap32(val);
}

uint64_t get_uint64(fb_tracker * fbt) {
    uint8_t len = 8;
    uint64_t val;

    // Check Highwater, reload buffer if needed
    if(fbt->buffer_pos + len > FILE_READ_BUFFER_SIZE) {
        relative_reset_buffer(fbt, FILE_READ_BUFFER_SIZE - fbt->buffer_pos);
    }

    val = *((uint64_t*)(fbt->buffer_ptr + fbt->buffer_pos));
    fbt->buffer_pos += len;

    return __builtin_bswap64(val);
}

int_least8_t get_int8(fb_tracker * fbt) {
    // Don't overflow the buffer, reload it.
    if(!get_remaining_bytes(fbt)) {
        load_buffer(fbt);
    }

    int_least8_t val = *((int_least8_t*)(fbt->buffer_ptr + fbt->buffer_pos));
    fbt->buffer_pos++;

    return val;
}

int16_t get_int16(fb_tracker * fbt) {
    uint8_t len = 2;
    int16_t val;

    // Check Highwater, reload buffer if needed
    if(fbt->buffer_pos + len > FILE_READ_BUFFER_SIZE) {
        relative_reset_buffer(fbt, FILE_READ_BUFFER_SIZE - fbt->buffer_pos);
    }

    val = *((int16_t*)(fbt->buffer_ptr + fbt->buffer_pos));
    fbt->buffer_pos += len;
    return __builtin_bswap16(val);
}

int32_t get_int32(fb_tracker * fbt) {
    uint8_t len = 4;
    int32_t val;

        // Check Highwater, reload buffer if needed
    if(fbt->buffer_pos + len > FILE_READ_BUFFER_SIZE) {
        relative_reset_buffer(fbt, FILE_READ_BUFFER_SIZE - fbt->buffer_pos);
    }

    val = *((int32_t*)(fbt->buffer_ptr + fbt->buffer_pos));
    fbt->buffer_pos += len;

    return __builtin_bswap32(val);
}

int64_t get_int64(fb_tracker * fbt) {
    uint8_t len = 8;
    int64_t val;

    // Check Highwater, reload buffer if needed
    if(fbt->buffer_pos + len > FILE_READ_BUFFER_SIZE) {
        relative_reset_buffer(fbt, FILE_READ_BUFFER_SIZE - fbt->buffer_pos);
    }

    val = *((int64_t*)(fbt->buffer_ptr + fbt->buffer_pos));
    fbt->buffer_pos += len;
    
    return __builtin_bswap64(val);
}

uint64_t get_varint(fb_tracker * fbt, uint8_t len) {
    int64_t val = 0;

    // Check Highwater, reload buffer if needed
    if((fbt->buffer_pos + len) > FILE_READ_BUFFER_SIZE) {
        relative_reset_buffer(fbt, FILE_READ_BUFFER_SIZE - fbt->buffer_pos);
    }

    memcpy(&val, (fbt->buffer_ptr + fbt->buffer_pos), len);
    fbt->buffer_pos += len;

    return __builtin_bswap64(val);
}

void get_string(fb_tracker * fbt, char * ptr, uint8_t len) {
    // Check Highwater, reload buffer if needed
    if(fbt->buffer_pos + len > FILE_READ_BUFFER_SIZE) {
        relative_reset_buffer(fbt, FILE_READ_BUFFER_SIZE - fbt->buffer_pos);
    }

    memcpy(ptr, (fbt->buffer_ptr + fbt->buffer_pos), len);
    fbt->buffer_pos+=len;    
    ptr[len] = '\0';
}

// LEB128 Decode
uint32_t get_vbe_uint(fb_tracker * fbt) {
    uint32_t val = 0;
    uint8_t shift = 0;

    while(shift <= 25) {
        uint8_t byte = get_uint8(fbt);
        val |= ((uint32_t)(byte & 0x7F)) << shift;
        if(!(byte & 0x80))
            break;
        shift += 7;
    }
    
    return val;
}

// LEB128 Signed Decode
int32_t get_vbe_int(fb_tracker * fbt) {
    int32_t val = 0;
    uint8_t shift = 0;
    
    uint8_t byte = get_uint8(fbt);

    while((byte & 0x80) && (shift <= 25)) {
        val |= ((uint32_t)(byte & 0x7f)) << shift;
        shift += 7;
        byte = get_uint8(fbt);
    }

    // Add sign bit
    if ((byte & 0x40)) {
        return -(val | ((uint32_t)(byte & 0x3f) << shift));
    }

    return val | ((uint32_t)(byte & 0x3f) << shift);
}
