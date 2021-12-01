#include <stdint.h>
#include <stdio.h>

#define FILE_READ_BUFFER_SIZE 1024  

typedef struct _file_buffer_handler {
    FILE * fp;
    uint16_t buffer_pos;
    uint16_t bytes_read;
    uint8_t buffer_ptr[FILE_READ_BUFFER_SIZE];
} fb_handler;

int init_buffer(fb_handler * fbh, char * filename);
void load_buffer(fb_handler * fbh);
void relative_reset_buffer(fb_handler * fbh, uint16_t seek);

static inline void file_seek(fb_handler * fbh, uint32_t offset) { 
    fseek(fbh->fp, offset, SEEK_SET);
    load_buffer(fbh);
};

static inline void file_seek_rel(fb_handler * fbh, uint32_t offset) { 
    fseek(fbh->fp, offset, SEEK_CUR);
    load_buffer(fbh);
};

static inline void file_close(fb_handler * fbh) { 
    fclose(fbh->fp); 
};