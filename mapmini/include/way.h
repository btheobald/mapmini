#ifndef WAY_GUARD
#define WAY_GUARD

#include <stdint.h>
#include <math.h>
#include "parse.h"
#include "memory.h"

typedef int16_t way_coord[2];

typedef struct _way_coord_blk { 
    uint8_t nodes;
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
    uint16_t *  tag_ids;
    uint8_t     flags;
    char *      name;
    char *      reference;
    char *      house;
    way_coord   label_off;
    uint8_t     blocks;
    way_data  * data;
} way_prop;

uint32_t get_way(way_prop * wp, fb_handler * fbh, arena_t * arena);

#define EARTH_R_M 6378137
#define SCALE 5

// Local spherical approximation
#define MD_RAD(X) (((float)X/1000000.0)*M_PI/180.0)

int32_t lat_to_y(int32_t lat);
int32_t lon_to_x(int32_t lon);

#endif
