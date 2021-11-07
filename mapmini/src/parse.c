#include "parse.h"

uint8_t get_uint8(fb_handler * fbh) {
    // Don't overflow the buffer, reload it.
    if(!get_remaining_bytes(fbh)) {
        load_buffer(fbh);
    } 
    
    uint8_t byte = *((uint8_t*)(fbh->buffer_ptr + fbh->buffer_pos));
    fbh->buffer_pos++;

    return byte;
}

uint16_t get_uint16(fb_handler * fbh) {
    uint8_t len = 2;
    uint16_t val;

    // Check Highwater, reload buffer if needed
    if(fbh->buffer_pos + len > FILE_READ_BUFFER_SIZE) {
        relative_reset_buffer(fbh, FILE_READ_BUFFER_SIZE - fbh->buffer_pos);
    }

    val = *((uint16_t*)(fbh->buffer_ptr + fbh->buffer_pos));
    fbh->buffer_pos += len;
    return __builtin_bswap16(val);
}

uint32_t get_uint32(fb_handler * fbh) {
    uint8_t len = 4;
    uint32_t val;

        // Check Highwater, reload buffer if needed
    if(fbh->buffer_pos + len > FILE_READ_BUFFER_SIZE) {
        relative_reset_buffer(fbh, FILE_READ_BUFFER_SIZE - fbh->buffer_pos);
    }

    val = *((uint32_t*)(fbh->buffer_ptr + fbh->buffer_pos));
    fbh->buffer_pos += len;

    return __builtin_bswap32(val);
}

uint64_t get_uint64(fb_handler * fbh) {
    uint8_t len = 8;
    uint64_t val;

    // Check Highwater, reload buffer if needed
    if(fbh->buffer_pos + len > FILE_READ_BUFFER_SIZE) {
        relative_reset_buffer(fbh, FILE_READ_BUFFER_SIZE - fbh->buffer_pos);
    }

    val = *((uint64_t*)(fbh->buffer_ptr + fbh->buffer_pos));
    fbh->buffer_pos += len;

    return __builtin_bswap64(val);
}

int_least8_t get_int8(fb_handler * fbh) {
    // Don't overflow the buffer, reload it.
    if(!get_remaining_bytes(fbh)) {
        load_buffer(fbh);
    }

    int_least8_t val = *((int_least8_t*)(fbh->buffer_ptr + fbh->buffer_pos));
    fbh->buffer_pos++;

    return val;
}

int16_t get_int16(fb_handler * fbh) {
    uint8_t len = 2;
    int16_t val;

    // Check Highwater, reload buffer if needed
    if(fbh->buffer_pos + len > FILE_READ_BUFFER_SIZE) {
        relative_reset_buffer(fbh, FILE_READ_BUFFER_SIZE - fbh->buffer_pos);
    }

    val = *((int16_t*)(fbh->buffer_ptr + fbh->buffer_pos));
    fbh->buffer_pos += len;
    return __builtin_bswap16(val);
}

int32_t get_int32(fb_handler * fbh) {
    uint8_t len = 4;
    int32_t val;

        // Check Highwater, reload buffer if needed
    if(fbh->buffer_pos + len > FILE_READ_BUFFER_SIZE) {
        relative_reset_buffer(fbh, FILE_READ_BUFFER_SIZE - fbh->buffer_pos);
    }

    val = *((int32_t*)(fbh->buffer_ptr + fbh->buffer_pos));
    fbh->buffer_pos += len;

    return __builtin_bswap32(val);
}

int64_t get_int64(fb_handler * fbh) {
    uint8_t len = 8;
    int64_t val;

    // Check Highwater, reload buffer if needed
    if(fbh->buffer_pos + len > FILE_READ_BUFFER_SIZE) {
        relative_reset_buffer(fbh, FILE_READ_BUFFER_SIZE - fbh->buffer_pos);
    }

    val = *((int64_t*)(fbh->buffer_ptr + fbh->buffer_pos));
    fbh->buffer_pos += len;
    
    return __builtin_bswap64(val);
}

uint64_t get_varint(fb_handler * fbh, uint8_t len) {
    uint8_t buffer[8];
    uint64_t val = 0;

    // Check Highwater, reload buffer if needed
    if((fbh->buffer_pos + len) > FILE_READ_BUFFER_SIZE) {
        relative_reset_buffer(fbh, FILE_READ_BUFFER_SIZE - fbh->buffer_pos);
    }

    memcpy(&buffer, (fbh->buffer_ptr + fbh->buffer_pos), len);
    fbh->buffer_pos += len;

    for(int i = 0; i < len; i++) {
        val |= ((buffer[i] &0xffL) << 8*(len-i-1)); 
    }

    return val;
}

// LEB128 Decode
uint32_t get_vbe_uint(fb_handler * fbh) {
    uint32_t val = 0;
    uint8_t shift = 0;

    while(shift <= 25) {
        uint8_t byte = get_uint8(fbh);
        val |= ((uint32_t)(byte & 0x7F)) << shift;
        if(!(byte & 0x80))
            break;
        shift += 7;
    }
    
    return val;
}

// LEB128 Signed Decode
int32_t get_vbe_int(fb_handler * fbh) {
    int32_t val = 0;
    uint8_t shift = 0;
    
    uint8_t byte = get_uint8(fbh);

    while((byte & 0x80) && (shift <= 25)) {
        val |= ((uint32_t)(byte & 0x7f)) << shift;
        shift += 7;
        byte = get_uint8(fbh);
    }

    // Add sign bit
    if ((byte & 0x40)) {
        return -(val | ((uint32_t)(byte & 0x3f) << shift));
    }

    return val | ((uint32_t)(byte & 0x3f) << shift);
}

void get_string(fb_handler * fbh, char * ptr, uint8_t len) {
    // Check Highwater, reload buffer if needed
    if(fbh->buffer_pos + len > FILE_READ_BUFFER_SIZE) {
        relative_reset_buffer(fbh, FILE_READ_BUFFER_SIZE - fbh->buffer_pos);
    }

    memcpy(ptr, (fbh->buffer_ptr + fbh->buffer_pos), len);
    fbh->buffer_pos+=len;    
    ptr[len] = '\0';
}