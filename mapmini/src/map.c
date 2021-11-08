#include <string.h>
#include <math.h>

#include "map.h"
#include "parse.h"
#include "way.h"
#include "memory.h"
#include <unistd.h>

//#include "hagl.h"
//#include "hagl_hal.h"

#define MAPSFORGE_MAGIC_STRING "mapsforge binary OSM"

int long2tilex(double lon, int z) { 
	return (int)(floor((lon + 180.0) / 360.0 * (1 << z))); 
}

int lat2tiley(double lat, int z) { 
    double latrad = lat * M_PI/180.0;
	return (int)(floor((1.0 - asinh(tan(latrad)) / M_PI) / 2.0 * (1 << z))); 
}

void g_draw_way(way_prop * way, uint8_t colour, uint8_t layer) {
    // I don't know why multiblocks cause problems
    //if(way->tag_ids[0] == layer) {
    //    for(uint16_t way_data = 0; way_data < way->blocks; way_data++) {
    //        for(uint16_t way_block = 0; way_block < way->data[way_data].polygons; way_block++) {
                //if(way->data[way_data].block[way_block].nodes >= 1) {
                //    hagl_draw_polygon(way->data[way_data].block[way_block].nodes, (int16_t*)(way->data[way_data].block[way_block].coords), hagl_color(255,255,255));
                //}
    //        }
    //    }
    //}
    if(way->data[0].block[0].nodes >= 1) {
        hagl_draw_polygon(way->data[0].block[0].nodes, (int16_t*)(way->data[0].block[0].coords), hagl_color(255,255,255));
    }   
}

int load_map(char* filename, uint32_t x_in, uint32_t y_in, uint32_t z_in) {
    fb_handler fbh;
    if(init_buffer(&fbh, "scotland_roads.map")) {
        return 1;
    }

    printf("Read in %d Bytes\n\r", fbh.bytes_read);

    if(memcmp(fbh.buffer_ptr, MAPSFORGE_MAGIC_STRING, 20)) {
        printf("Not a valid .MAP file!\n\r");
        return -1;
    } else {
        printf("Valid .MAP: %s\n\r", fbh.buffer_ptr);
    }

    arena_t a0;
    arena_init(&a0, ARENA_DEFAULT_SIZE);

    fbh.buffer_pos += 20;

    mapsforge_file_header hdr;

    hdr.header_size = get_uint32(&fbh);
    printf("Header Size:%d\n\r", hdr.header_size);
    hdr.file_version = get_uint32(&fbh);
    printf("File Version:%d\n\r", hdr.file_version);
    hdr.file_size = get_uint64(&fbh);
    printf("File Size:%uMB\n\r", (uint8_t)(hdr.file_size/1000000));
    hdr.file_creation = get_int64(&fbh);
    printf("File Created:%llu\n\r", (uint64_t)hdr.file_creation/1000);
    hdr.bounding_box[0] = get_int32(&fbh);
    hdr.bounding_box[1] = get_int32(&fbh);
    hdr.bounding_box[2] = get_int32(&fbh);
    hdr.bounding_box[3] = get_int32(&fbh);
    printf("Bounding Box:\n\r");
    printf("\t[0]:%7.3f\n\r\t[1]:%7.3f\n\r\t[2]:%7.3f\n\r\t[3]:%7.3f\n\r", (float)hdr.bounding_box[0]/1000000, (float)hdr.bounding_box[1]/1000000, (float)hdr.bounding_box[2]/1000000, (float)hdr.bounding_box[3]/1000000);
    hdr.tile_size = get_uint16(&fbh);
    printf("Tile Size:\t%hhu\n\r", hdr.tile_size);

    uint8_t str_len = get_uint8(&fbh);
    get_string(&fbh, hdr.projection,str_len);
    printf("Projection:\t%s\n\r", hdr.projection);

    hdr.flags = get_uint8(&fbh);

    if(hdr.flags & 0x40) {
        hdr.init_lat_long[0] = get_uint32(&fbh);
        hdr.init_lat_long[1] = get_uint32(&fbh);
        printf("Start Position:\n\r");
        printf("\t[0]:%7.3f\n\r\t[1]:%7.3f\n\r", (float)hdr.init_lat_long[0]/1000000, (float)hdr.init_lat_long[1]/1000000);
    } else {
        hdr.init_lat_long[0] = 0;
        hdr.init_lat_long[1] = 0;
    }

    if(hdr.flags & 0x20) {
        hdr.init_zoom = get_uint8(&fbh);    
        printf("Start Zoom:\t%u\n\r", hdr.init_zoom);
    } else {
        hdr.init_zoom = 0;
    }

    if(hdr.flags & 0x10) {
        str_len = get_uint8(&fbh);
        get_string(&fbh, hdr.lang_pref,str_len);
        printf("Language:\t%s\n\r", hdr.lang_pref); 
    } else {
        hdr.lang_pref[0] = '\0';
    }

    if(hdr.flags & 0x08) {
        str_len = get_uint8(&fbh);
        get_string(&fbh, hdr.comment, str_len);    
        printf("Comment:\t%s\n\r", fbh.buffer_pos, hdr.comment);
    } else {
        hdr.comment[0] = '\0';
    }

    if(hdr.flags & 0x04) {
        str_len = get_uint8(&fbh);
        get_string(&fbh, hdr.created_by, str_len);
        printf("Created By:\t%s\n\n\r", hdr.created_by);
    } else {
        hdr.created_by[0] = '\0';
    }

    hdr.n_poi_tags = get_uint16(&fbh);

    printf("# of POI Tags:\t%hhu\n\r", hdr.n_poi_tags);

    for(int poi_id = 0; poi_id < hdr.n_poi_tags; poi_id++) {
        str_len = get_uint8(&fbh);
        get_string(&fbh, hdr.poi_tag_names[poi_id], str_len);
        //printf("\t[%d]: %d : %s\n", poi_id, str_len, hdr.poi_tag_names[poi_id]);
    }

    hdr.n_way_tags = get_uint16(&fbh);

    printf("# of Way Tags:\t%d\n\r", hdr.n_way_tags);
 
    for(int way_id = 0; way_id < hdr.n_way_tags; way_id++) {
        str_len = get_uint8(&fbh);
        get_string(&fbh, hdr.way_tag_names[way_id], str_len);
        printf("\t[%d]: %s\n\r", way_id, hdr.way_tag_names[way_id]);
    }

    hdr.n_zoom_intervals = get_uint8(&fbh);

    printf("\n# Zoom Intervals:\t%u\n\r", hdr.n_zoom_intervals);

    for(int zoom_id = 0; zoom_id < hdr.n_zoom_intervals; zoom_id++) {
        hdr.zoom_conf[zoom_id].base_zoom = get_uint8(&fbh);
        hdr.zoom_conf[zoom_id].min_zoom = get_uint8(&fbh);
        hdr.zoom_conf[zoom_id].max_zoom = get_uint8(&fbh);
        hdr.zoom_conf[zoom_id].sub_file = get_uint64(&fbh);
        hdr.zoom_conf[zoom_id].sub_file_size = get_uint64(&fbh);

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
    


    int z_ds;
    for(z_ds = 0; z_ds < hdr.n_zoom_intervals; z_ds++)
        if(z_in > hdr.zoom_conf[z_ds].min_zoom & \
           z_in < hdr.zoom_conf[z_ds].max_zoom) break;

    printf("Zoom Interval:%d\n\r", z_ds);    

    uint32_t x_ds = x_in - long2tilex(((double) hdr.bounding_box[1])/1000000, hdr.zoom_conf[z_ds].base_zoom);
    uint32_t y_ds = y_in - lat2tiley(((double) hdr.bounding_box[2])/1000000, hdr.zoom_conf[z_ds].base_zoom);

    uint32_t t_lookup = ((y_ds*hdr.zoom_conf[z_ds].n_tiles_x) + x_ds);

    file_seek(&fbh, hdr.zoom_conf[z_ds].sub_file+(t_lookup*5));
    uint64_t offset_lookup = get_varint(&fbh, 5) & 0x7fffffffff;
    
    printf("%u/%d/%d -> %lu, %llu\n\r", hdr.zoom_conf[z_ds].base_zoom, x_in, y_in, t_lookup, offset_lookup);
    
    file_seek(&fbh, hdr.zoom_conf[z_ds].sub_file+offset_lookup);

    uint16_t pois[22] = {0};
    uint16_t ways[22] = {0};

    printf("Z\tPOIs\tWays\n\r");
    for(int z = hdr.zoom_conf[z_ds].min_zoom; z <= hdr.zoom_conf[z_ds].max_zoom; z++) {
        pois[z] = get_vbe_uint(&fbh);
        ways[z] = get_vbe_uint(&fbh);
        printf("%d\t%d\t%d\n\r", z, pois[z], ways[z]);
    }
    printf("Zoom Table End\n\r");
    
    uint32_t first_way_offset = get_vbe_uint(&fbh);
    uint32_t first_way_file_addr = hdr.zoom_conf[z_ds].sub_file + \
                                         offset_lookup + \
                                         fbh.buffer_pos + \
                                         first_way_offset;

    printf("First Way Offset: %lu - %lu\n\r", first_way_offset, first_way_file_addr);
    file_seek(&fbh, first_way_file_addr);                                   

    const int ways_to_draw = ways[12]+ways[13]+ways[14];//+ways[15];
    way_prop testway[ways_to_draw];
    uint32_t way_size = 0;
    for(int w = 0; w < ways_to_draw; w++) {
        printf("LOAD ", w);
        way_size += get_way(&testway[w],&fbh,&a0);
        if(testway[w].subtile_bitmap & 0xFFFF)
        printf("DRAW ", w);
        g_draw_way(&testway[w], 0, testway[w].tag_ids[0]);
    }

    printf("Size of Ways: %d\n\r", way_size);
    
    printf("Arena: %d/%d\n\r", arena_free(&a0), ARENA_DEFAULT_SIZE);

    file_close(&fbh);

    return 0;
}
