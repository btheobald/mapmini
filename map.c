#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "map.h"
#include "file.h"
#include "graphics.h"
#include "way.h"
#include "mem.h"
#include "ff.h"

#define MAPSFORGE_MAGIC_STRING "mapsforge binary OSM"

typedef struct _mapsforge_zoom_interval {
    uint8_t base_zoom;
    uint8_t max_zoom;
    uint8_t min_zoom;
    uint64_t sub_file;
    uint64_t sub_file_size;
    uint16_t n_tiles_x;
    uint16_t n_tiles_y;
} mapsforge_zoom_interval;

typedef struct _mapsforge_file_header {
    uint32_t header_size;
    uint32_t file_version;
    uint64_t file_size; 
    uint64_t file_creation; // Milliseconds!
    int32_t bounding_box[4];
    uint16_t tile_size;
    char projection[16];
    uint8_t flags;
    int32_t init_lat_long[2];
    uint8_t init_zoom;
    char lang_pref[32];
    char comment[40];
    char created_by[40];
    uint16_t n_poi_tags;
    char poi_tag_names[100][32];
    uint16_t n_way_tags;
    char way_tag_names[360][32];
    uint8_t n_zoom_intervals;
    mapsforge_zoom_interval zoom_conf[3];
} mapsforge_file_header;

int long2tilex(double lon, int z) { 
	return (int)(floor((lon + 180.0) / 360.0 * (1 << z))); 
}

int lat2tiley(double lat, int z) { 
    double latrad = lat * M_PI/180.0;
	return (int)(floor((1.0 - asinh(tan(latrad)) / M_PI) / 2.0 * (1 << z))); 
}

int render_map(char* filename) {
    FIL mf;

    uint8_t buffer[FILE_READ_BUFFER_SIZE];

    printf("Opening: %s\n\r", filename);
    
    if(f_open(&mf, filename, FA_READ) != FR_OK) {
        printf("Failed\n\r");
    }

    g_init();
    g_fill(255);

    // Copy Sizable Buffer for header
    fb_tracker fbt;
    init_buffer(&mf, &fbt, buffer);

    printf("Read in %d Bytes\n\r", fbt.bytes_read);

    init_arena(65535); // 64KB Buffer

    if(memcmp(fbt.buffer_ptr, MAPSFORGE_MAGIC_STRING, 20)) {
        printf("Not a valid .MAP file!\n\r");
        return -1;
    } else {
        printf("Valid .MAP: %s\n\r", fbt.buffer_ptr);
    }

    fbt.buffer_pos += 20;

    mapsforge_file_header hdr;

    hdr.header_size = get_uint32(&fbt);
    printf("Header Size:%d\n\r", hdr.header_size);
    hdr.file_version = get_uint32(&fbt);
    printf("File Version:%d\n\r", hdr.file_version);
    hdr.file_size = get_uint64(&fbt);
    printf("File Size:%uMB\n\r", (uint8_t)(hdr.file_size/1000000));
    hdr.file_creation = get_int64(&fbt);
    printf("File Created:%llu\n\r", (uint64_t)hdr.file_creation/1000);
    hdr.bounding_box[0] = get_int32(&fbt);
    hdr.bounding_box[1] = get_int32(&fbt);
    hdr.bounding_box[2] = get_int32(&fbt);
    hdr.bounding_box[3] = get_int32(&fbt);
    printf("Bounding Box:\n\r");
    printf("\t[0]:%7.3f\n\r\t[1]:%7.3f\n\r\t[2]:%7.3f\n\r\t[3]:%7.3f\n\r", (float)hdr.bounding_box[0]/1000000, (float)hdr.bounding_box[1]/1000000, (float)hdr.bounding_box[2]/1000000, (float)hdr.bounding_box[3]/1000000);
    hdr.tile_size = get_uint16(&fbt);
    printf("Tile Size:\t%hhu\n\r", hdr.tile_size);

    uint8_t str_len = get_uint8(&fbt);
    get_string(&fbt, hdr.projection,str_len);
    printf("Projection:\t%s\n\r", hdr.projection);

    hdr.flags = get_uint8(&fbt);

    if(hdr.flags & 0x40) {
        hdr.init_lat_long[0] = get_uint32(&fbt);
        hdr.init_lat_long[1] = get_uint32(&fbt);
        printf("Start Position:\n\r");
        printf("\t[0]:%7.3f\n\r\t[1]:%7.3f\n\r", (float)hdr.init_lat_long[0]/1000000, (float)hdr.init_lat_long[1]/1000000);
    } else {
        hdr.init_lat_long[0] = 0;
        hdr.init_lat_long[1] = 0;
    }

    if(hdr.flags & 0x20) {
        hdr.init_zoom = get_uint8(&fbt);    
        printf("Start Zoom:\t%u\n\r", hdr.init_zoom);
    } else {
        hdr.init_zoom = 0;
    }

    if(hdr.flags & 0x10) {
        str_len = get_uint8(&fbt);
        get_string(&fbt, hdr.lang_pref,str_len);
        printf("Language:\t%s\n\r", hdr.lang_pref); 
    } else {
        hdr.lang_pref[0] = '\0';
    }

    if(hdr.flags & 0x08) {
        str_len = get_uint8(&fbt);
        get_string(&fbt, hdr.comment, str_len);    
        printf("Comment:\t%s\n\r", fbt.buffer_pos, hdr.comment);
    } else {
        hdr.comment[0] = '\0';
    }

    if(hdr.flags & 0x04) {
        str_len = get_uint8(&fbt);
        get_string(&fbt, hdr.created_by, str_len);
        printf("Created By:\t%s\n\n\r", hdr.created_by);
    } else {
        hdr.created_by[0] = '\0';
    }

    hdr.n_poi_tags = get_uint16(&fbt);

    printf("# of POI Tags:\t%hhu\n\r", hdr.n_poi_tags);

    for(int poi_id = 0; poi_id < hdr.n_poi_tags; poi_id++) {
        str_len = get_uint8(&fbt);
        get_string(&fbt, hdr.poi_tag_names[poi_id], str_len);
        //printf("\t[%d]: %d : %s\n", poi_id, str_len, hdr.poi_tag_names[poi_id]);
    }

    hdr.n_way_tags = get_uint16(&fbt);

    printf("# of Way Tags:\t%d\n\r", hdr.n_way_tags);
 
    for(int way_id = 0; way_id < hdr.n_way_tags; way_id++) {
        str_len = get_uint8(&fbt);
        get_string(&fbt, hdr.way_tag_names[way_id], str_len);
        printf("\t[%d]: %s\n\r", way_id, hdr.way_tag_names[way_id]);
    }

    hdr.n_zoom_intervals = get_uint8(&fbt);

    printf("\n# Zoom Intervals:\t%d\n\r", fbt.buffer_pos, hdr.n_zoom_intervals);

    for(int zoom_id = 0; zoom_id < hdr.n_zoom_intervals; zoom_id++) {
        hdr.zoom_conf[zoom_id].base_zoom = get_uint8(&fbt);
        hdr.zoom_conf[zoom_id].min_zoom = get_uint8(&fbt);
        hdr.zoom_conf[zoom_id].max_zoom = get_uint8(&fbt);
        hdr.zoom_conf[zoom_id].sub_file = get_uint64(&fbt);
        hdr.zoom_conf[zoom_id].sub_file_size = get_uint64(&fbt);

        hdr.zoom_conf[zoom_id].n_tiles_x = (long2tilex(((double)hdr.bounding_box[3])/1000000, hdr.zoom_conf[zoom_id].base_zoom) - long2tilex(((double)hdr.bounding_box[1])/1000000, hdr.zoom_conf[zoom_id].base_zoom)) + 1;
        hdr.zoom_conf[zoom_id].n_tiles_y = (lat2tiley(((double)hdr.bounding_box[0])/1000000, hdr.zoom_conf[zoom_id].base_zoom) - lat2tiley(((double)hdr.bounding_box[2])/1000000, hdr.zoom_conf[zoom_id].base_zoom)) + 1;

        printf("Zoom Interval [%d]:\n\r", zoom_id);
        printf("\tBase Zoom: %u\n\r", hdr.zoom_conf[zoom_id].base_zoom);
        printf("\tMax Zoom: %u\n\r", hdr.zoom_conf[zoom_id].max_zoom);
        printf("\tMin Zoom: %u\n\r", hdr.zoom_conf[zoom_id].min_zoom);
        printf("\tSub-file Start: %llu\n\r", hdr.zoom_conf[zoom_id].sub_file);
        printf("\tSub-file Size: %fMB\n\r", (float)hdr.zoom_conf[zoom_id].sub_file_size/1000000);
        printf("\t# of Tiles in X: %u\n\r", hdr.zoom_conf[zoom_id].n_tiles_x);
        printf("\t# of Tiles in Y: %u\n\r", hdr.zoom_conf[zoom_id].n_tiles_y);
        printf("\t# of Tiles: %u\n\r", hdr.zoom_conf[zoom_id].n_tiles_y * hdr.zoom_conf[zoom_id].n_tiles_x );
        printf("\tOSM Base Tile Origin: %d/%d/%d\n\r", hdr.zoom_conf[zoom_id].base_zoom, long2tilex(((double)hdr.bounding_box[1])/1000000, hdr.zoom_conf[zoom_id].base_zoom), lat2tiley(((double)hdr.bounding_box[2])/1000000, hdr.zoom_conf[zoom_id].base_zoom));
    }
    
    uint32_t x_in = 8046;
    uint32_t y_in = 5105;
    uint32_t z_in = 14;

    int z_ds;
    for(z_ds = 0; z_ds < hdr.n_zoom_intervals; z_ds++)
        if(z_in > hdr.zoom_conf[z_ds].min_zoom & \
           z_in < hdr.zoom_conf[z_ds].max_zoom) break;

    printf("Zoom Interval:%d\n\r", z_ds);    

    uint32_t x_ds = x_in - long2tilex(((double) hdr.bounding_box[1])/1000000, hdr.zoom_conf[z_ds].base_zoom);
    uint32_t y_ds = y_in - lat2tiley(((double) hdr.bounding_box[2])/1000000, hdr.zoom_conf[z_ds].base_zoom);

    uint32_t t_lookup = ((y_ds*hdr.zoom_conf[z_ds].n_tiles_x) + x_ds);

    f_lseek(fbt.fp, hdr.zoom_conf[z_ds].sub_file+(t_lookup*5));

    char offset_buffer[5];    
    f_read(fbt.fp, offset_buffer, 5, 1);
    int64_t raw_lookup = (offset_buffer[0] & 0xffL) << 32 | \
                          (offset_buffer[1] & 0xffL) << 24 | \
                          (offset_buffer[2] & 0xffL) << 16 | \
                          (offset_buffer[3] & 0xffL) <<  8 | \
                          (offset_buffer[4] & 0xffL);
    uint64_t offset_lookup = (raw_lookup & 0x7fffffffff);
    
    printf("%u/%d/%d = %lu -> %lu\n\r", hdr.zoom_conf[z_ds].base_zoom, x_in, y_in, raw_lookup, offset_lookup);
    
    f_lseek(fbt.fp, hdr.zoom_conf[z_ds].sub_file+offset_lookup);
    load_buffer(&fbt);

    uint16_t pois[22] = {0};
    uint16_t ways[22] = {0};

    printf("Z\tPOIs\tWays\n\r");
    for(int z = hdr.zoom_conf[z_ds].min_zoom; z <= hdr.zoom_conf[z_ds].max_zoom; z++) {
        pois[z] = get_vbe_uint(&fbt);
        ways[z] = get_vbe_uint(&fbt);
        printf("%d\t%d\t%d\n\r", z, pois[z], ways[z]);
    }

    uint32_t first_way_offset = get_vbe_uint(&fbt);
    uint32_t first_way_file_addr = hdr.zoom_conf[z_ds].sub_file + \
                                         offset_lookup + \
                                         fbt.buffer_pos + \
                                         first_way_offset;

    f_lseek(fbt.fp, first_way_file_addr);                                   
    load_buffer(&fbt);

    const int ways_to_draw = ways[12]+ways[13]+ways[14];
    way_prop testway[ways_to_draw];
    uint32_t way_size = 0;
    for(int w = 0; w < ways_to_draw; w++) {
        way_size += get_way(&testway[w],&fbt);
        //printf("%d ", w);
        g_draw_way(&testway[w], 0, testway[w].tag_ids[0]);
        //printf("Drawn\n\r", w);
    }

    printf("Size of Ways: %d\n\r", way_size);

    free_arena();
    
    f_close(&mf);

    return 0;
}
