#ifndef GRAPHICS_GUARD
#define GRAPHICS_GUARD

#include <stdint.h>
#include <math.h>
#include "way.h"

#define SCREEN_X 480
#define SCREEN_Y 272

int g_init();
void g_fill(uint8_t colour);
void g_pixel(int16_t x, int16_t y, uint8_t colour);
void g_line(int16_t x0, int16_t x1, int16_t y0, int16_t y1, uint8_t colour);
void g_poly(way_coord_blk * block, uint8_t colour);
void g_poly_fill(way_coord_blk * block, uint8_t colour);
void g_poly_line(way_coord_blk * block, uint8_t colour);

void g_draw_way(way_prop * way, uint8_t colour, uint8_t layer);

#endif
