#include <stdio.h>
#include <stdint.h>
#include "io_posix.h"
#include "memory.h"
#include "way.h"

typedef struct {
    uint8_t base_zoom;
    uint8_t max_zoom;
    uint8_t min_zoom;
    uint64_t sub_file;
    uint64_t sub_file_size;
    uint16_t n_tiles_x;
    uint16_t n_tiles_y;
} mm_zoom_interval_t;

typedef struct {
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
    mm_zoom_interval_t zoom_conf[3];
} mm_file_header_t;

typedef struct {
    uint32_t x;
    uint32_t y;
    uint32_t z;
} mm_tile_coord_t;

typedef struct {
    uint32_t t_lookup;
    uint16_t pois[22];
    uint16_t ways[22];
    way_prop * way_data;
    uint16_t number_of_ways;
} mm_tile_header;

typedef struct {
    int32_t x_offset;
    int32_t y_offset;
    int32_t scale;
    int32_t rotation_sin;
    int32_t rotation_cos;
    uint16_t subtile;
} mm_view_params_t;

#define FIXED_POINT_SCALE 1000000//65536 // 2 ^ 16 (< 1E-3 fractional rotational resolution and fast shift division)

#define TILE_ADDR_MASK 0x7fffffffffULL
#define TILE_WATER_MASK 0x8000000000ULL

int load_map_file(fb_handler * fbh, char * filename);
int load_map_header(fb_handler * fbh, mm_file_header_t * hdr);
int load_map_tile(fb_handler * fbh, mm_file_header_t * hdr, mm_tile_header * tile, mm_tile_coord_t * xyz, mm_view_params_t * view, arena_t * arena);
int long2tilex(double lon, int z);
int lat2tiley(double lat, int z);
int tilex2long(int x, int z);
int tiley2lat(int y, int z);