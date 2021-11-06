#include "way.h"
#include "memory.h"

uint32_t get_way(way_prop * wp, fb_handler * fbh) {
    uint32_t ds = get_vbe_uint(fbh);

    printf("Size: %d -  ", ds);
    uint32_t start = fbh->bytes_read;

    if(fbh->buffer_pos + ds > FILE_READ_BUFFER_SIZE) {
        relative_reset_buffer(fbh, FILE_READ_BUFFER_SIZE - fbh->buffer_pos);
    }

    wp->subtile_bitmap = get_uint16(fbh);
    printf("Subtile: %u -  ", wp->subtile_bitmap);

    uint8_t special = get_uint8(fbh);
    wp->osm_layer = (special & 0xf0) >> 4;
    printf("Layer: %d -  ", wp->osm_layer);
    wp->n_tags = (special & 0x0f);

    wp->tag_ids = (uint16_t*)mark_bytes(sizeof(uint32_t)*wp->n_tags);
    printf("Tags: %d -  ", wp->n_tags);

    for(int tag = 0; tag < wp->n_tags; tag++) {
        wp->tag_ids[tag] = get_vbe_uint(fbh);
        printf("%d - ", wp->tag_ids[tag]);
    }

    wp->flags = get_uint8(fbh);

    if(wp->flags & 0x80) { // Way Name
        uint8_t len = get_uint8(fbh);
        wp->name = (char*)mark_bytes(sizeof(char)*len+1);
        get_string(fbh, wp->name, len);
        printf("Name: %s, ", wp->name);
    }
    if(wp->flags & 0x40) { // House Number
        uint8_t len = get_uint8(fbh);
        wp->house = (char*)mark_bytes(sizeof(char)*len+1);
        get_string(fbh, wp->house, len);
        printf("House: %s, ", wp->house);
    }
    if(wp->flags & 0x20) { // Reference
        uint8_t len = get_uint8(fbh);
        wp->reference = (char*)mark_bytes(sizeof(char)*len+1);
        get_string(fbh, wp->reference, len);
        printf("Ref: %s, ", wp->reference);
    }
    if(wp->flags & 0x10) { // Label Position
        wp->label_off.xy[0] = get_vbe_int(fbh);
        wp->label_off.xy[1] = get_vbe_int(fbh);
        printf("LabelPos %d %d ",  wp->label_off.xy[0],  wp->label_off.xy[1]);
    }
    if(wp->flags & 0x08) { // Number of Way Data Blocks
        wp->blocks = (uint8_t)get_vbe_uint(fbh);
    } else {
        wp->blocks = 1;
    }
    wp->data = (way_data*)mark_bytes(sizeof(way_data)*wp->blocks);

    printf("%d Blocks ", wp->blocks);

    for(int wdb = 0; wdb < wp->blocks; wdb++) {
        wp->data[wdb].polygons = (uint32_t)get_vbe_uint(fbh);
        printf("%d Poly ", wp->data[wdb].polygons);
        wp->data[wdb].block = (way_coord_blk*)mark_bytes(sizeof(way_coord_blk)*wp->data[wdb].polygons);
        uint32_t last_lon_md, last_lat_md;
        for(int wcb = 0; wcb < wp->data[wdb].polygons; wcb++) {
            wp->data[wdb].block[wcb].nodes = (uint32_t)get_vbe_uint(fbh);
            printf("%d Nodes ", wp->data[wdb].block[wcb].nodes);
            wp->data[wdb].block[wcb].coords = (way_coord*)mark_bytes(sizeof(way_coord)*wp->data[wdb].block[wcb].nodes);
            
            // Get Origin
            last_lon_md = get_vbe_int(fbh);
            last_lat_md = get_vbe_int(fbh);

            wp->data[wdb].block[wcb].coords[0].xy[0] = abs(lon_to_x(last_lon_md));
            wp->data[wdb].block[wcb].coords[0].xy[1] = abs(lat_to_y(last_lat_md));

            // Loop Coordinates
            if(!(wp->flags & 0x04)) { // Single Delta
                for(int wc = 1; wc < wp->data[wdb].block[wcb].nodes; wc++) {
                    last_lon_md += get_vbe_int(fbh);
                    last_lat_md += get_vbe_int(fbh);
                    wp->data[wdb].block[wcb].coords[wc].xy[0] = abs(lon_to_x(last_lon_md));
                    wp->data[wdb].block[wcb].coords[wc].xy[1] = abs(lat_to_y(last_lat_md));
                }
            } else { // Double Delta
                uint32_t dx = 0;
                uint32_t dy = 0;
                for(int wc = 1; wc < wp->data[wdb].block[wcb].nodes; wc++) {
                    dx += get_vbe_int(fbh);
                    dy += get_vbe_int(fbh);
                    last_lon_md += dx;
                    last_lat_md += dy;
                    wp->data[wdb].block[wcb].coords[wc].xy[0] = abs(lon_to_x(last_lon_md));
                    wp->data[wdb].block[wcb].coords[wc].xy[1] = abs(lat_to_y(last_lat_md));
                }
            }
        }
    }

    printf("\n");

    return ds;
}

int32_t lon_to_x(int32_t lon) {
    return (MD_RAD(lon)*EARTH_R_M) * 1.72 / SCALE;
}

int32_t lat_to_y(int32_t lat) {
    return (MD_RAD(lat)*EARTH_R_M) / SCALE;
}
