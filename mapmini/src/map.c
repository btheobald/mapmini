#include <string.h>
#include <math.h>

#include "map.h"
#include "parse.h"
#include "way.h"
#include "memory.h"
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

float tilex2long(int x, int z) 
{
	return x / (float)(1 << z) * 360.0 - 180;
}

float tiley2lat(int y, int z) 
{
	float n = M_PI - 2.0 * M_PI * y / (float)(1 << z);
	return 180.0 / M_PI * atan(0.5 * (exp(n) - exp(-n)));
}

void g_draw_way(way_prop * way, uint8_t colour, uint8_t layer, int16_t xo, int16_t yo, float rot, uint16_t size) {

    if(way->data[0].block[0].nodes > 1) {
      
        uint16_t cl = hagl_hal_color(100,100,100);
        uint8_t th = 1;

        for(uint8_t t = 0; t < way->n_tags; t++) {
            switch(((uint8_t*)way->tag_ids)[t]) {
                case 26: // Pedestrian
                case 13: // Steps
                    cl = hagl_hal_color(0xE5,0xE0,0xC2);
                    th = 1;
                    goto tag_found;
                case 3: // Footway
                case 4: // Path
                    cl = hagl_hal_color(0xAA,0x00,0x00);
                    th = 1;
                    goto tag_found;
                case 2: // Track
                    cl = hagl_hal_color(0xFF,0xFA,0xF2);
                    th = 1;
                    goto tag_found;                    
                case 14: // Cycleway
                    cl = hagl_hal_color(0xFF,0xF2,0xDE);
                    th = 1;
                    goto tag_found;
                case 32: // Bridleway
                    cl = hagl_hal_color(0xD3,0xCB,0x98);
                    th = 1;
                    goto tag_found;           
                case 0: // Service
                    cl = hagl_hal_color(0xFF,0xFF,0xFF);
                    th = 1;
                    goto tag_found;          
                case 28: // Construction
                    cl = hagl_hal_color(0xD0,0xD0,0xD0);
                    th = 1;
                    goto tag_found;    
                case 64: // Road
                    cl = hagl_hal_color(0xD0,0xD0,0xD0);
                    th = 2;
                    goto tag_found;              
                case 1: // Residential
                case 6: // Unclassified
                case 30: // Living Street
                    cl = hagl_hal_color(0xFF,0xFF,0xFF);
                    th = 2;
                    goto tag_found;         
                case 8:  // Tertiary
                case 35: // Tertiary Link
                    cl = hagl_hal_color(0xFF,0xFF,0x90);
                    th = 3;
                    goto tag_found;    
                case 12: // Secondary
                case 34: // Secondary Link
                    cl = hagl_hal_color(0xBB,0x85,0x0F);
                    th = 3;
                    goto tag_found;
                case 7: // Primary
                    cl = hagl_hal_color(0xFE,0x85,0x0C);
                    th = 4;
                    goto tag_found;
                case 27: // Primary Link
                    cl = hagl_hal_color(0xFE,0x85,0x0C);
                    th = 3;
                    goto tag_found;   
                case 11: // Trunk
                    cl = hagl_hal_color(0x80,0x00,0x40);
                    th = 4;
                    goto tag_found;
                case 24: // Trunk Link
                    cl = hagl_hal_color(0x80,0x00,0x40);
                    th = 3;
                    goto tag_found;    
                case 21: // Motorway
                    cl = hagl_hal_color(0x40,0x00,0x00);
                    th = 3;
                    goto tag_found;
                case 23: // Motorway Link
                    cl = hagl_hal_color(0x40,0x00,0x00);
                    th = 3;
                    goto tag_found;  
                //default:
                    //printf("Tag %d Not Found: %hu\n", t, way->tag_ids[t]);
            } 
        }
        tag_found:

        // Rotate Data
        for(int p = 0; p < way->data[0].block[0].nodes; p++) {
            int32_t xt = xo+way->data[0].block[0].coords[p][0]-DISPLAY_WIDTH/2;
            int32_t yt = yo+way->data[0].block[0].coords[p][1]-DISPLAY_HEIGHT/2;
            //printf("%d, %d, %d, %d", xt, yt, xt*cos(1)-yt*sin(1), yt*cos(1)+xt*sin(1));
            way->data[0].block[0].coords[p][0] = xt*cos(rot)-yt*sin(rot)+DISPLAY_WIDTH/2;
            way->data[0].block[0].coords[p][1] = yt*cos(rot)+xt*sin(rot)+DISPLAY_HEIGHT/2;
        }

        for(int i = 0; i < (way->data[0].block[0].nodes-1); i++) {
            if(!(way->data[0].block[0].coords[i][0] == way->data[0].block[0].coords[i+1][0] && way->data[0].block[0].coords[i][1] == way->data[0].block[0].coords[i+1][1])){

            if(cl != 0) draw_varthick_line( way->data[0].block[0].coords[i][0], 
                                            way->data[0].block[0].coords[i][1],
                                            way->data[0].block[0].coords[i+1][0],
                                            way->data[0].block[0].coords[i+1][1],
                                            th, cl);
            }

            /*if(cl != 0) hagl_draw_line(way->data[0].block[0].coords[i][0], 
                                       way->data[0].block[0].coords[i][1],
                                       way->data[0].block[0].coords[i+1][0],
                                       way->data[0].block[0].coords[i+1][1], cl);
            } */         
        }
    }   
}

int load_map(char* filename, uint32_t x_in, uint32_t y_in, uint32_t z_in, int16_t xo, int16_t yo, uint16_t st, float rot, float size) {
    fb_handler fbh;
    if(init_buffer(&fbh, "scotland_roads.map")) {
        return 1;
    }

    //printf("Read in %d Bytes\n\r", fbh.bytes_read);

    if(memcmp(fbh.buffer_ptr, MAPSFORGE_MAGIC_STRING, 20)) {
        printf("Not a valid .MAP file!\n\r");
        return -1;
    } else {
        //printf("Valid .MAP: %s\n\r", fbh.buffer_ptr);
    }

    arena_t a0;
    arena_init(&a0, ARENA_DEFAULT_SIZE);

    fbh.buffer_pos += 20;

    mapsforge_file_header hdr;

    hdr.header_size = get_uint32(&fbh);
    //printf("Header Size:%d\n\r", hdr.header_size);
    hdr.file_version = get_uint32(&fbh);
    //printf("File Version:%d\n\r", hdr.file_version);
    hdr.file_size = get_uint64(&fbh);
    //printf("File Size:%uMB\n\r", (uint8_t)(hdr.file_size/1000000));
    hdr.file_creation = get_int64(&fbh);
    //printf("File Created:%llu\n\r", (uint64_t)hdr.file_creation/1000);
    hdr.bounding_box[0] = get_int32(&fbh);
    hdr.bounding_box[1] = get_int32(&fbh);
    hdr.bounding_box[2] = get_int32(&fbh);
    hdr.bounding_box[3] = get_int32(&fbh);
    //printf("Bounding Box:\n\r");
    //printf("\t[0]:%7.3f\n\r\t[1]:%7.3f\n\r\t[2]:%7.3f\n\r\t[3]:%7.3f\n\r", (float)hdr.bounding_box[0]/1000000, (float)hdr.bounding_box[1]/1000000, (float)hdr.bounding_box[2]/1000000, (float)hdr.bounding_box[3]/1000000);
    hdr.tile_size = get_uint16(&fbh);
    //printf("Tile Size:\t%hhu\n\r", hdr.tile_size);

    uint8_t str_len = get_uint8(&fbh);
    get_string(&fbh, hdr.projection,str_len);
    //printf("Projection:\t%s\n\r", hdr.projection);

    hdr.flags = get_uint8(&fbh);

    if(hdr.flags & 0x40) {
        hdr.init_lat_long[0] = get_uint32(&fbh);
        hdr.init_lat_long[1] = get_uint32(&fbh);
        //printf("Start Position:\n\r");
        //printf("\t[0]:%7.3f\n\r\t[1]:%7.3f\n\r", (float)hdr.init_lat_long[0]/1000000, (float)hdr.init_lat_long[1]/1000000);
    } else {
        hdr.init_lat_long[0] = 0;
        hdr.init_lat_long[1] = 0;
    }

    if(hdr.flags & 0x20) {
        hdr.init_zoom = get_uint8(&fbh);    
        //printf("Start Zoom:\t%u\n\r", hdr.init_zoom);
    } else {
        hdr.init_zoom = 0;
    }

    if(hdr.flags & 0x10) {
        str_len = get_uint8(&fbh);
        get_string(&fbh, hdr.lang_pref,str_len);
        //printf("Language:\t%s\n\r", hdr.lang_pref); 
    } else {
        hdr.lang_pref[0] = '\0';
    }

    if(hdr.flags & 0x08) {
        str_len = get_uint8(&fbh);
        get_string(&fbh, hdr.comment, str_len);    
        //printf("Comment:\t%s\n\r", fbh.buffer_pos, hdr.comment);
    } else {
        hdr.comment[0] = '\0';
    }

    if(hdr.flags & 0x04) {
        str_len = get_uint8(&fbh);
        get_string(&fbh, hdr.created_by, str_len);
        //printf("Created By:\t%s\n\n\r", hdr.created_by);
    } else {
        hdr.created_by[0] = '\0';
    }

    hdr.n_poi_tags = get_uint16(&fbh);

    //printf("# of POI Tags:\t%hhu\n\r", hdr.n_poi_tags);

    for(int poi_id = 0; poi_id < hdr.n_poi_tags; poi_id++) {
        str_len = get_uint8(&fbh);
        get_string(&fbh, hdr.poi_tag_names[poi_id], str_len);
        //printf("\t[%d]: %d : %s\n", poi_id, str_len, hdr.poi_tag_names[poi_id]);
    }

    hdr.n_way_tags = get_uint16(&fbh);

    //printf("# of Way Tags:\t%d\n\r", hdr.n_way_tags);
 
    for(int way_id = 0; way_id < hdr.n_way_tags; way_id++) {
        str_len = get_uint8(&fbh);
        get_string(&fbh, hdr.way_tag_names[way_id], str_len);
        //printf("\t[%d]: %s\n\r", way_id, hdr.way_tag_names[way_id]);
    }

    hdr.n_zoom_intervals = get_uint8(&fbh);

    //printf("\n# Zoom Intervals:\t%u\n\r", hdr.n_zoom_intervals);

    for(int zoom_id = 0; zoom_id < hdr.n_zoom_intervals; zoom_id++) {
        hdr.zoom_conf[zoom_id].base_zoom = get_uint8(&fbh);
        hdr.zoom_conf[zoom_id].min_zoom = get_uint8(&fbh);
        hdr.zoom_conf[zoom_id].max_zoom = get_uint8(&fbh);
        hdr.zoom_conf[zoom_id].sub_file = get_uint64(&fbh);
        hdr.zoom_conf[zoom_id].sub_file_size = get_uint64(&fbh);

        hdr.zoom_conf[zoom_id].n_tiles_x = (long2tilex(((double)hdr.bounding_box[3])/1000000, hdr.zoom_conf[zoom_id].base_zoom) - long2tilex(((double)hdr.bounding_box[1])/1000000, hdr.zoom_conf[zoom_id].base_zoom)) + 1;
        hdr.zoom_conf[zoom_id].n_tiles_y = (lat2tiley(((double)hdr.bounding_box[0])/1000000, hdr.zoom_conf[zoom_id].base_zoom) - lat2tiley(((double)hdr.bounding_box[2])/1000000, hdr.zoom_conf[zoom_id].base_zoom)) + 1;

        //printf("Zoom Interval [%d]:\n\r", zoom_id);
        //printf("\tBase Zoom: %u\n\r", hdr.zoom_conf[zoom_id].base_zoom);
        //printf("\tMax Zoom: %u\n\r", hdr.zoom_conf[zoom_id].max_zoom);
        //printf("\tMin Zoom: %u\n\r", hdr.zoom_conf[zoom_id].min_zoom);
        //printf("\tSub-file Start: %llu\n\r", hdr.zoom_conf[zoom_id].sub_file);
        //printf("\tSub-file Size: %fMB\n\r", (float)hdr.zoom_conf[zoom_id].sub_file_size/1000000);
        //printf("\t# of Tiles in X: %u\n\r", hdr.zoom_conf[zoom_id].n_tiles_x);
        //printf("\t# of Tiles in Y: %u\n\r", hdr.zoom_conf[zoom_id].n_tiles_y);
        //printf("\t# of Tiles: %u\n\r", hdr.zoom_conf[zoom_id].n_tiles_y * hdr.zoom_conf[zoom_id].n_tiles_x );
        //printf("\tOSM Base Tile Origin: %d/%d/%d\n\r", hdr.zoom_conf[zoom_id].base_zoom, long2tilex(((double)hdr.bounding_box[1])/1000000, hdr.zoom_conf[zoom_id].base_zoom), lat2tiley(((double)hdr.bounding_box[2])/1000000, hdr.zoom_conf[zoom_id].base_zoom));
    }
    
    int z_ds;
    for(z_ds = 0; z_ds < hdr.n_zoom_intervals; z_ds++)
        if(z_in > hdr.zoom_conf[z_ds].min_zoom & \
           z_in < hdr.zoom_conf[z_ds].max_zoom) break;

    //printf("Zoom Interval:%d\n\r", z_ds);    

    uint32_t x_ds = x_in - long2tilex(((double) hdr.bounding_box[1])/1000000, hdr.zoom_conf[z_ds].base_zoom);
    uint32_t y_ds = y_in - lat2tiley(((double) hdr.bounding_box[2])/1000000, hdr.zoom_conf[z_ds].base_zoom);

    uint32_t t_lookup = ((y_ds*hdr.zoom_conf[z_ds].n_tiles_x) + x_ds);

    const uint64_t addr_mask =  0x7fffffffffULL;
    const uint64_t water_mask = 0x8000000000ULL;

    file_seek(&fbh, hdr.zoom_conf[z_ds].sub_file+(t_lookup*5));
    uint64_t addr_lookup = get_varint(&fbh, 5);
    uint64_t offset_lookup = addr_lookup & addr_mask;

    if(addr_lookup & water_mask) {
      //printf("Only Water\n");
      //printf("Arena: %d/%d\n\r", arena_free(&a0), ARENA_DEFAULT_SIZE);
      file_close(&fbh);
      return 0;
    }
        
    //printf("%u/%d/%d -> %lu, %llu, %llu\n\r", hdr.zoom_conf[z_ds].base_zoom, x_in, y_in, t_lookup, offset_lookup, addr_lookup&water_mask);
    
    file_seek(&fbh, hdr.zoom_conf[z_ds].sub_file+offset_lookup);

    uint16_t pois[22] = {0};
    uint16_t ways[22] = {0};

    //printf("Z\tPOIs\tWays\n\r");
    for(int z = hdr.zoom_conf[z_ds].min_zoom; z <= hdr.zoom_conf[z_ds].max_zoom; z++) {
        pois[z] = get_vbe_uint(&fbh);
        ways[z] = get_vbe_uint(&fbh);
        //printf("%d\t%d\t%d\n\r", z, pois[z], ways[z]);
    }
    //printf("Zoom Table End\n\r");
    
    uint32_t first_way_offset = get_vbe_uint(&fbh);
    uint32_t first_way_file_addr = hdr.zoom_conf[z_ds].sub_file + \
                                         offset_lookup + \
                                         fbh.buffer_pos + \
                                         first_way_offset;

    //printf("First Way Offset: %lu - %lu\n\r", first_way_offset, first_way_file_addr);
    file_seek(&fbh, first_way_file_addr);                                   

    int ways_to_draw = ways[12]+ways[13]+ways[14]+ways[15];
    way_prop testway[ways_to_draw];
    uint32_t way_size = 0;
    
    double lon = tilex2long(x_in,z_in);
    double lat = tiley2lat(y_in,z_in);
  
    double lon1 = tilex2long(x_in+1,z_in);
    double lat1 = tiley2lat(y_in+1,z_in);
  
    double londiff = fabs(lon1-lon);
    double latdiff = fabs(lat1-lat);
  
    //printf("tile   origin: %f, %f\n", lat, lon);
    //printf("tile + origin: %f, %f\n", lat1, lon1);
    //printf("tile differences: %f, %f\n", fabs(lat1-lat), fabs(lon1-lon));
    
    int x_pix = lon_to_x(londiff*1000000, 1);
    int y_pix = lat_to_y(latdiff*1000000, 1);
    float x_mercator = ((float)x_pix/y_pix);
    float fit_scale = y_pix/size;
    int x_fit = lon_to_x(londiff*1000000, x_mercator*fit_scale);
    int y_fit = lat_to_y(latdiff*1000000, fit_scale);
    
    //printf("scale diff to: %d, %d\n", y_pix, x_pix);
    //printf("scale factors: %f, %f (%f)\n", fit_scale, x_mercator*fit_scale, x_mercator);
    //printf("fit diff tile: %d, %d\n", y_fit, x_fit);
      
    for(int w = 0; w < ways_to_draw; w++) {
        way_size = get_way(&testway[w],&fbh,&a0, st, fit_scale, x_mercator);
        if(st & testway[w].subtile_bitmap)
            g_draw_way(&testway[w], 0, testway[w].tag_ids[0], xo+DISPLAY_WIDTH/2, yo+DISPLAY_HEIGHT/2, rot, size);
    }

    //printf("Size of Ways: %d\n\r", way_size);
    
    //printf("Arena: %d/%d\n\r", arena_free(&a0), ARENA_DEFAULT_SIZE);

    file_close(&fbh);

    return arena_free(&a0);
}
