#include <string.h>
#include <math.h>

#include "map.h"
#include "parse.h"
#include "way.h"
#include <unistd.h>

#include "hagl.h"
#include "hagl_hal.h"
#include "thick.h"

#define MAPSFORGE_MAGIC_STRING "mapsforge binary OSM"

int long2tilex(double lon, int z) { 
	return (int)(floor((lon + 180.0) / 360.0 * (1 << z))); 
}

int lat2tiley(double lat, int z) { 
    double latrad = lat * M_PI/180.0;
	return (int)(floor((1.0 - asinh(tan(latrad)) / M_PI) / 2.0 * (1 << z))); 
}

int tilex2long(int x, int z) 
{
	return FIXED_POINT_SCALE * (x / (float)(1 << z) * 360.0 - 180);
}

int tiley2lat(int y, int z) 
{
	float n = M_PI - (2.0 * M_PI * y) / (1 << z);
	return FIXED_POINT_SCALE * 180.0 / M_PI * atan(0.5 * (exp(n) - exp(-n)));
}

void g_draw_way(way_prop * way, color_t colour, uint8_t layer, mm_view_params_t * view) {

    if(way->data[0].block[0].nodes > 1) {
      
        color_t cl = hagl_color(100,100,100);
        uint8_t th = 1;

        for(uint8_t t = 0; t < way->n_tags; t++) {
            switch(((uint8_t*)way->tag_ids)[t]) {
                case 26: // Pedestrian
                case 13: // Steps
                    cl = hagl_color(0xE5,0xE0,0xC2);
                    th = 1;
                    goto tag_found;
                case 3: // Footway
                case 4: // Path
                    cl = hagl_color(0xAA,0x00,0x00);
                    th = 1;
                    goto tag_found;
                case 2: // Track
                    cl = hagl_color(0xFF,0xFA,0xF2);
                    th = 1;
                    goto tag_found;                    
                case 14: // Cycleway
                    cl = hagl_color(0xFF,0xF2,0xDE);
                    th = 1;
                    goto tag_found;
                case 32: // Bridleway
                    cl = hagl_color(0xD3,0xCB,0x98);
                    th = 1;
                    goto tag_found;           
                case 0: // Service
                    cl = hagl_color(0xFF,0xFF,0xFF);
                    th = 1;
                    goto tag_found;          
                case 28: // Construction
                    cl = hagl_color(0xD0,0xD0,0xD0);
                    th = 1;
                    goto tag_found;    
                case 64: // Road
                    cl = hagl_color(0xD0,0xD0,0xD0);
                    th = 2;
                    goto tag_found;              
                case 1: // Residential
                case 6: // Unclassified
                case 30: // Living Street
                    cl = hagl_color(0xFF,0xFF,0xFF);
                    th = 2;
                    goto tag_found;         
                case 8:  // Tertiary
                case 35: // Tertiary Link
                    cl = hagl_color(0xFF,0xFF,0x90);
                    th = 3;
                    goto tag_found;    
                case 12: // Secondary
                case 34: // Secondary Link
                    cl = hagl_color(0xBB,0x85,0x0F);
                    th = 3;
                    goto tag_found;
                case 7: // Primary
                    cl = hagl_color(0xFE,0x85,0x0C);
                    th = 4;
                    goto tag_found;
                case 27: // Primary Link
                    cl = hagl_color(0xFE,0x85,0x0C);
                    th = 3;
                    goto tag_found;   
                case 11: // Trunk
                    cl = hagl_color(0x80,0x00,0x40);
                    th = 4;
                    goto tag_found;
                case 24: // Trunk Link
                    cl = hagl_color(0x80,0x00,0x40);
                    th = 3;
                    goto tag_found;    
                case 21: // Motorway
                    cl = hagl_color(0x40,0x00,0x00);
                    th = 3;
                    goto tag_found;
                case 23: // Motorway Link
                    cl = hagl_color(0x40,0x00,0x00);
                    th = 3;
                    goto tag_found;  
                //default:
                    //("Tag %d Not Found: %hu\n", t, way->tag_ids[t]);
            } 
        }
        tag_found:

        // Rotate Data
        for(int p = 0; p < way->data[0].block[0].nodes; p++) {
            int32_t xt = view->x_offset+way->data[0].block[0].coords[p].x-DISPLAY_WIDTH/2;
            int32_t yt = view->y_offset+way->data[0].block[0].coords[p].y-DISPLAY_HEIGHT/2;
            way->data[0].block[0].coords[p].x = ((xt*view->rotation_cos)-(yt*view->rotation_sin)+FIXED_POINT_SCALE*DISPLAY_WIDTH/2)/FIXED_POINT_SCALE;
            way->data[0].block[0].coords[p].y = ((yt*view->rotation_cos)+(xt*view->rotation_sin)+FIXED_POINT_SCALE*DISPLAY_HEIGHT/2)/FIXED_POINT_SCALE;
        }

        for(int i = 0; i < (way->data[0].block[0].nodes-1); i++) {
            if(!(way->data[0].block[0].coords[i].x == way->data[0].block[0].coords[i+1].x && way->data[0].block[0].coords[i].y == way->data[0].block[0].coords[i+1].y)){
            if(cl != 0) draw_varthick_line( way->data[0].block[0].coords[i].x, 
                                            way->data[0].block[0].coords[i].y,
                                            way->data[0].block[0].coords[i+1].x,
                                            way->data[0].block[0].coords[i+1].y,
                                            th, cl);
            }    
        }
    }   
}

int load_map_file(fb_handler * fbh, char * filename) {
    if(init_buffer(fbh, filename)) {
        return 1;
    }
    printf("Read in %d Bytes\n\r", fbh->bytes_read);
}

void print_map_header(mm_file_header_t * hdr) {
    printf("Header Size:%d\n\r", hdr->header_size);
    printf("File Version:%d\n\r", hdr->file_version);
    printf("File Size:%uMB\n\r", (uint8_t)(hdr->file_size/1000000));
    printf("File Created:%llu\n\r", (uint64_t)hdr->file_creation/1000);
    printf("Bounding Box:\n\r");
    printf("\t[0]:%7.3f\n\r\t[1]:%7.3f\n\r\t[2]:%7.3f\n\r\t[3]:%7.3f\n\r", (float)hdr->bounding_box[0]/1000000, (float)hdr->bounding_box[1]/1000000, (float)hdr->bounding_box[2]/1000000, (float)hdr->bounding_box[3]/1000000);
    printf("Tile Size:\t%hhu\n\r", hdr->tile_size);
    printf("Projection:\t%s\n\r", hdr->projection);

    if(hdr->flags & 0x40)
        printf("Start Position: \t[0]:%7.3f\n\r\t[1]:%7.3f\n\r", (float)hdr->init_lat_long[0]/1000000, (float)hdr->init_lat_long[1]/1000000);

    if(hdr->flags & 0x20)
        printf("Start Zoom:\t%u\n\r", hdr->init_zoom);

    if(hdr->flags & 0x10)
        printf("Language:\t%s\n\r", hdr->lang_pref); 

    if(hdr->flags & 0x08)
        printf("Comment:\t%s\n\r", hdr->comment);

    if(hdr->flags & 0x04) 
        printf("Created By:\t%s\n\r", hdr->created_by);

    printf("# of POI Tags:\t%hhu\n\r", hdr->n_poi_tags);
    for(int poi_id = 0; poi_id < hdr->n_poi_tags; poi_id++)
        printf("\t[%d]: %s\n\r", poi_id, hdr->poi_tag_names[poi_id]);

    printf("# of Way Tags:\t%d\n\r", hdr->n_way_tags);
    for(int way_id = 0; way_id < hdr->n_way_tags; way_id++) {
        printf("\t[%d]: %s\n\r", way_id, hdr->way_tag_names[way_id]);
    }

    printf("\n# Zoom Intervals:\t%u\n\r", hdr->n_zoom_intervals);
    for(int zoom_id = 0; zoom_id < hdr->n_zoom_intervals; zoom_id++) {
        printf("Zoom Interval [%d]:\n\r", zoom_id);
        printf("\tBase Zoom: %u\n\r", hdr->zoom_conf[zoom_id].base_zoom);
        printf("\tMax Zoom: %u\n\r", hdr->zoom_conf[zoom_id].max_zoom);
        printf("\tMin Zoom: %u\n\r", hdr->zoom_conf[zoom_id].min_zoom);
        printf("\tSub-file Start: %llu\n\r", hdr->zoom_conf[zoom_id].sub_file);
        printf("\tSub-file Size: %fMB\n\r", (float)hdr->zoom_conf[zoom_id].sub_file_size/1000000);
        printf("\t# of Tiles in X: %u\n\r", hdr->zoom_conf[zoom_id].n_tiles_x);
        printf("\t# of Tiles in Y: %u\n\r", hdr->zoom_conf[zoom_id].n_tiles_y);
        printf("\t# of Tiles: %u\n\r", hdr->zoom_conf[zoom_id].n_tiles_y * hdr->zoom_conf[zoom_id].n_tiles_x );
        printf("\tOSM Base Tile Origin: %d/%d/%d\n\r", hdr->zoom_conf[zoom_id].base_zoom, long2tilex(((double)hdr->bounding_box[1])/1000000, hdr->zoom_conf[zoom_id].base_zoom), lat2tiley(((double)hdr->bounding_box[2])/1000000, hdr->zoom_conf[zoom_id].base_zoom));
    } 
}

int load_map_header(fb_handler * fbh, mm_file_header_t * hdr) {  
    if(memcmp(fbh->buffer_ptr, MAPSFORGE_MAGIC_STRING, 20)) {
        printf("Not a valid .MAP file!\n\r");
        return -1;
    } else {
        printf("Valid .MAP: %s\n\r", fbh->buffer_ptr);
    }

    fbh->buffer_pos += 20;

    hdr->header_size = get_uint32(fbh);
    hdr->file_version = get_uint32(fbh);
    hdr->file_size = get_uint64(fbh);
    hdr->file_creation = get_int64(fbh);
    hdr->bounding_box[0] = get_int32(fbh);
    hdr->bounding_box[1] = get_int32(fbh);
    hdr->bounding_box[2] = get_int32(fbh);
    hdr->bounding_box[3] = get_int32(fbh);

    hdr->tile_size = get_uint16(fbh);
    uint8_t str_len = get_uint8(fbh);
    get_string(fbh, hdr->projection,str_len);

    hdr->flags = get_uint8(fbh);
    if(hdr->flags & 0x40) {
        hdr->init_lat_long[0] = get_uint32(fbh);
        hdr->init_lat_long[1] = get_uint32(fbh);
    } else {
        hdr->init_lat_long[0] = 0;
        hdr->init_lat_long[1] = 0;
    }

    if(hdr->flags & 0x20) {
        hdr->init_zoom = get_uint8(fbh);
    } else {
        hdr->init_zoom = 0;
    }

    if(hdr->flags & 0x10) {
        str_len = get_uint8(fbh);
        get_string(fbh, hdr->lang_pref,str_len);
    } else {
        hdr->lang_pref[0] = '\0';
    }

    if(hdr->flags & 0x08) {
        str_len = get_uint8(fbh);
        get_string(fbh, hdr->comment, str_len);
    } else {
        hdr->comment[0] = '\0';
    }

    if(hdr->flags & 0x04) {
        str_len = get_uint8(fbh);
        get_string(fbh, hdr->created_by, str_len);
    } else {
        hdr->created_by[0] = '\0';
    }

    hdr->n_poi_tags = get_uint16(fbh);

    for(int poi_id = 0; poi_id < hdr->n_poi_tags; poi_id++) {
        str_len = get_uint8(fbh);
        get_string(fbh, hdr->poi_tag_names[poi_id], str_len);
    }

    hdr->n_way_tags = get_uint16(fbh);

    for(int way_id = 0; way_id < hdr->n_way_tags; way_id++) {
        str_len = get_uint8(fbh);
        get_string(fbh, hdr->way_tag_names[way_id], str_len);
    }

    hdr->n_zoom_intervals = get_uint8(fbh);

    for(int zoom_id = 0; zoom_id < hdr->n_zoom_intervals; zoom_id++) {
        hdr->zoom_conf[zoom_id].base_zoom = get_uint8(fbh);
        hdr->zoom_conf[zoom_id].min_zoom = get_uint8(fbh);
        hdr->zoom_conf[zoom_id].max_zoom = get_uint8(fbh);
        hdr->zoom_conf[zoom_id].sub_file = get_uint64(fbh);
        hdr->zoom_conf[zoom_id].sub_file_size = get_uint64(fbh);

        hdr->zoom_conf[zoom_id].n_tiles_x = (long2tilex(((double)hdr->bounding_box[3])/1000000, hdr->zoom_conf[zoom_id].base_zoom) - long2tilex(((double)hdr->bounding_box[1])/1000000, hdr->zoom_conf[zoom_id].base_zoom)) + 1;
        hdr->zoom_conf[zoom_id].n_tiles_y = (lat2tiley(((double)hdr->bounding_box[0])/1000000, hdr->zoom_conf[zoom_id].base_zoom) - lat2tiley(((double)hdr->bounding_box[2])/1000000, hdr->zoom_conf[zoom_id].base_zoom)) + 1;
    } 

    print_map_header(hdr);
}

void print_map_tile() {

}

int load_map_tile(fb_handler * fbh, mm_file_header_t * hdr, mm_tile_header * tile, mm_tile_coord_t * xyz, mm_view_params_t * view, arena_t * arena) {
    // Lookup closest zoom_interval
    int z_interval;
    for(z_interval = 0; z_interval < hdr->n_zoom_intervals; z_interval++)
        if(xyz->z > hdr->zoom_conf[z_interval].min_zoom & \
           xyz->z < hdr->zoom_conf[z_interval].max_zoom) break;

    // Tile address offset calculation
    uint32_t x_ds = xyz->x - long2tilex(((double) hdr->bounding_box[1])/1000000, hdr->zoom_conf[z_interval].base_zoom);
    uint32_t y_ds = xyz->y - lat2tiley(((double) hdr->bounding_box[2])/1000000, hdr->zoom_conf[z_interval].base_zoom);
    uint32_t t_lookup = ((y_ds*hdr->zoom_conf[z_interval].n_tiles_x) + x_ds);

    // Seek to tile offset
    file_seek(fbh, hdr->zoom_conf[z_interval].sub_file+(t_lookup*5));
    uint64_t addr_lookup = get_varint(fbh, 5);
    uint64_t offset_lookup = addr_lookup & TILE_ADDR_MASK;

    // Check if tile is empty (Address is the same as next tile) or is only water.
    if((addr_lookup == get_varint(fbh, 5) & TILE_ADDR_MASK) || (addr_lookup & TILE_WATER_MASK)) {
      return 1;
    }
    
    // Seek to tile.
    file_seek(fbh, hdr->zoom_conf[z_interval].sub_file+offset_lookup);

    // POI / Way Z Table
    for(int z = hdr->zoom_conf[z_interval].min_zoom; z <= hdr->zoom_conf[z_interval].max_zoom; z++) {
        tile->pois[z] = get_vbe_uint(fbh);
        tile->ways[z] = get_vbe_uint(fbh);
    }
    
    // File Offset of First Way
    uint32_t first_way_offset = get_vbe_uint(fbh);
    uint32_t first_way_file_addr = hdr->zoom_conf[z_interval].sub_file + \
                                         offset_lookup + \
                                         fbh->buffer_pos + \
                                         first_way_offset;

    // Seek to first way.
    file_seek(fbh, first_way_file_addr);                                   

    // Number of Ways at zoom level
    tile->number_of_ways = tile->ways[12];//+tile->ways[13]+tile->ways[14]+tile->ways[15];

    // Allocate maximum ways for tile.
    tile->way_data = malloc(tile->number_of_ways * sizeof(way_prop));

    for(int w = 0; w < tile->number_of_ways; w++) {
        uint8_t rtn = get_way(&(tile->way_data[w]), fbh, arena, view->subtile);
        if(rtn) { // Ignore way
          if(w > 0) w--; 
          tile->number_of_ways--;  
        }
    }
    for(int w = 0; w < tile->number_of_ways; w++) {
        if(view->subtile & tile->way_data[w].subtile_bitmap)
            g_draw_way(&(tile->way_data[w]), 0, tile->way_data[w].tag_ids[0], view);
    }
}
