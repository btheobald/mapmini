#include "way.h"
#include "memory.h"
#include <time.h>

uint32_t get_way(way_prop * wp, fb_handler * fbh, arena_t * arena, uint16_t st, float scale, float x_mercator) {
    uint32_t ds = get_vbe_uint(fbh);
    //printf("Size: %d -  ", ds);

    if(fbh->buffer_pos + ds > FILE_READ_BUFFER_SIZE) {
        relative_reset_buffer(fbh, FILE_READ_BUFFER_SIZE - fbh->buffer_pos);
    }

    wp->subtile_bitmap = get_uint16(fbh);
    //printf("Subtile: %04X -  ", wp->subtile_bitmap);

    //if(st & wp->subtile_bitmap) {
    //    return ds-2;
    //}

    uint8_t special = get_uint8(fbh);
    wp->osm_layer = (special & 0xf0) >> 4;
    //printf("Layer: %d -  ", wp->osm_layer);
    wp->n_tags = (special & 0x0f);

    wp->tag_ids = (uint16_t*)arena_malloc(arena,sizeof(uint16_t)*wp->n_tags);
    //printf("Tags: %d -  ", wp->n_tags);

    for(int tag = 0; tag < wp->n_tags; tag++) {
        wp->tag_ids[tag] = get_vbe_uint(fbh);
        //printf("%d - ", wp->tag_ids[tag]);
    }

    wp->flags = get_uint8(fbh);

    if(wp->flags & 0x80) { // Way Name
        uint8_t len = get_uint8(fbh);
        wp->name = (char*)arena_malloc(arena,sizeof(char)*len+1);
        get_string(fbh, wp->name, len);
        //printf("Name: %s, ", wp->name);
    }
    if(wp->flags & 0x40) { // House Number
        uint8_t len = get_uint8(fbh);
        wp->house = (char*)arena_malloc(arena,sizeof(char)*len+1);
        get_string(fbh, wp->house, len);
        //printf("House: %s, ", wp->house);
    }
    if(wp->flags & 0x20) { // Reference
        uint8_t len = get_uint8(fbh);
        wp->reference = (char*)arena_malloc(arena,sizeof(char)*len+1);
        get_string(fbh, wp->reference, len);
        //printf("Ref: %s, ", wp->reference);
    }
    if(wp->flags & 0x10) { // Label Position
        wp->label_off[0] = get_vbe_int(fbh);
        wp->label_off[1] = get_vbe_int(fbh);
        //printf("LabelPos %d %d ",  wp->label_off[0],  wp->label_off[1]);
    }
    if(wp->flags & 0x08) { // Number of Way Data Blocks
        wp->blocks = (uint8_t)get_vbe_uint(fbh);
    } else {
        wp->blocks = 1;
    }
    wp->data = (way_data*)arena_malloc(arena,sizeof(way_data)*wp->blocks);

    //printf("%d Blocks ", wp->blocks);

    for(int wdb = 0; wdb < wp->blocks; wdb++) {
        wp->data[wdb].polygons = (uint32_t)get_vbe_uint(fbh);
        //printf("%d Polygons ", wp->data[wdb].polygons);
        wp->data[wdb].block = (way_coord_blk*)arena_malloc(arena,sizeof(way_coord_blk)*wp->data[wdb].polygons);
        int32_t last_lon_md, last_lat_md;
        for(int wcb = 0; wcb < wp->data[wdb].polygons; wcb++) {
            wp->data[wdb].block[wcb].nodes = (uint32_t)get_vbe_uint(fbh);
            //printf("%d Nodes ", wp->data[wdb].block[wcb].nodes);
            wp->data[wdb].block[wcb].coords = (way_coord*)arena_malloc(arena,sizeof(way_coord)*wp->data[wdb].block[wcb].nodes);
            
            // Get Origin
            last_lon_md = get_vbe_int(fbh);
            last_lat_md = get_vbe_int(fbh);

            wp->data[wdb].block[wcb].coords[0][0] = abs(lat_to_y(last_lat_md, scale*x_mercator));
            wp->data[wdb].block[wcb].coords[0][1] = abs(lon_to_x(last_lon_md, scale));
            //printf("%d ", wp->data[wdb].block[wcb].coords[0][0]);
            //printf("%d ", wp->data[wdb].block[wcb].coords[0][1]);

            // Loop Coordinates
            if(!(wp->flags & 0x04)) { // Single Delta
                for(int wc = 1; wc < wp->data[wdb].block[wcb].nodes; wc++) {
                    last_lon_md += get_vbe_int(fbh);
                    last_lat_md += get_vbe_int(fbh);
                    wp->data[wdb].block[wcb].coords[wc][0] = abs(lat_to_y(last_lat_md, scale*x_mercator));
                    wp->data[wdb].block[wcb].coords[wc][1] = abs(lon_to_x(last_lon_md, scale));
                    //printf("%d ", wp->data[wdb].block[wcb].coords[wc][0]);
                    //printf("%d ", wp->data[wdb].block[wcb].coords[wc][1]);
                }
            } else { // Double Delta
                int32_t dx = 0;
                int32_t dy = 0;
                for(int wc = 1; wc < wp->data[wdb].block[wcb].nodes; wc++) {
                    dx += get_vbe_int(fbh);
                    dy += get_vbe_int(fbh);
                    last_lon_md += dx;
                    last_lat_md += dy;
                    wp->data[wdb].block[wcb].coords[wc][0] = abs(lat_to_y(last_lat_md, scale*x_mercator));
                    wp->data[wdb].block[wcb].coords[wc][1] = abs(lon_to_x(last_lon_md, scale));
                    //printf("%d ", wp->data[wdb].block[wcb].coords[wc][0]);
                    //printf("%d ", wp->data[wdb].block[wcb].coords[wc][1]);
                }
            }
        }
        //printf("Block\n");
    }
    //printf("\n");

    return 0; // No advance needed
}

int32_t lon_to_x(int32_t lon, float scale) {
    return (MD_RAD(lon)*EARTH_R_M) / scale;
}

int32_t lat_to_y(int32_t lat, float scale) {
    return (MD_RAD(lat)*EARTH_R_M) / scale;
}
