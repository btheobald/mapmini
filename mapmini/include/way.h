#ifndef WAY_GUARD
#define WAY_GUARD

#include <stdint.h>
#include <math.h>
#include "parse.h"
#include "memory.h"

typedef struct _way_coord {
    int16_t x;
    int16_t y;
} way_coord;

typedef struct _way_coord_blk { 
    uint16_t nodes;
    way_coord * coords; 
} way_coord_blk;

typedef struct _way_data {
    uint8_t polygons;
    way_coord_blk * block;
} way_data;

typedef struct _way_prop {
    uint16_t    subtile_bitmap;
    uint8_t     osm_layer;
    uint8_t     n_tags;
    uint8_t *  tag_ids;
    uint8_t     flags;
    char *      name;
    char *      reference;
    char *      house;
    way_coord   label_off;
    uint8_t     blocks;
    way_data  * data;
} way_prop;

uint32_t get_way(way_prop * wp, fb_handler * fbh, arena_t * arena, uint16_t st, float scale, float x_mercator);

#define EARTH_R_M 6378137
#define SCALE 6

// Local spherical approximation
#define MD_RAD(X) (((float)X/1000000.0)*M_PI/180.0)

int32_t lat_to_y(int32_t lat, float scale);
int32_t lon_to_x(int32_t lon, float scale);

#endif
