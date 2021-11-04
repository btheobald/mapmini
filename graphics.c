#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include "graphics.h"
#include <unistd.h>
#include "way.h"
#include "stm32h7b3i_discovery_conf.h"
#include "stm32_lcd.h"

void* fb;
int fb_file;
uint32_t* fb_val;

int g_init() {
    fb_val = LCD_LAYER_0_ADDRESS;
}

void g_fill(uint8_t colour) {
    UTIL_LCD_Clear(colour);
}

void g_pixel(int16_t x, int16_t y, uint8_t colour) {
    if((x > 0) && (x < SCREEN_X) && (y > 0) && (y < SCREEN_Y)) {
        //fb_val[y+x*SCREEN_Y] = UTIL_LCD_COLOR_BLACK;
        UTIL_LCD_SetPixel(x,SCREEN_Y-y,UTIL_LCD_COLOR_BLACK);
    }
}

void g_line(int16_t x0, int16_t x1, int16_t y0, int16_t y1, uint8_t colour) {
    int16_t dx = abs(x1 - x0);
    int8_t sx = x0 < x1 ? 1:-1;
    int16_t dy = -abs(y1 - y0);
    int8_t sy = y0 < y1 ? 1:-1;
    int16_t err = dx+dy;

    while(1) {
        g_pixel(x0, y0, colour);
        if(x0==x1 && y0==y1) break;
        int32_t e2 = 2*err;
        if(e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if(e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void g_poly(way_coord_blk * block, uint8_t colour) {
    int i;
    for(i = 0; i < block->nodes-1; i++) {
        g_line(block->coords[i].xy[0],block->coords[i+1].xy[0],block->coords[i].xy[1],block->coords[i+1].xy[1],colour);
    }
    g_line(block->coords[i].xy[0],block->coords[0].xy[0],block->coords[i].xy[1],block->coords[0].xy[1],colour);
}

void g_poly_fill(way_coord_blk * block, uint8_t colour) {
    int32_t  nodes, nodeX[block->nodes], pixelX, pixelY, i, j, swap ;

    //  Loop through the rows of the image.
    for (pixelY=0; pixelY<SCREEN_Y; pixelY++) {
        //  Build a list of nodes.
        nodes=0; j=block->nodes-1;
        for (i=0; i<block->nodes; i++) {
            if (((block->coords[i].xy[1] < pixelY) && (block->coords[j].xy[1] >= pixelY)) || ((block->coords[j].xy[1] < pixelY) && (block->coords[i].xy[1] >= pixelY))) {
                nodeX[nodes++] = (block->coords[i].xy[0] + (pixelY-block->coords[i].xy[1])*(block->coords[j].xy[0]-block->coords[i].xy[0])/(block->coords[j].xy[1]-block->coords[i].xy[1])); 
            }
            j=i;
        }

        //  Sort the nodes, via a simple “Bubble” sort.
        i=0;
        while (i<nodes-1) {
            if (nodeX[i  ] > nodeX[i+1]) {
                swap       = nodeX[i  ]; 
                nodeX[i  ] = nodeX[i+1]; 
                nodeX[i+1] =swap; 
                if (i) i--;
            } else {
                i++; 
            }
        }

        //  Fill the pixels between node pairs.
        for (i=0; i<nodes; i+=2) {
            if (nodeX[i] >= SCREEN_Y) break;
            if (nodeX[i+1] > 0) {
                if (nodeX[i] < 0) nodeX[i] = 0 ;
                    if (nodeX[i+1] > SCREEN_X) nodeX[i+1] = SCREEN_X;
                        for (pixelX = nodeX[i]; pixelX<nodeX[i+1]; pixelX++) 
                            g_pixel(pixelX,pixelY,colour); 
            }
        }
    }
}

void g_poly_line(way_coord_blk * block, uint8_t colour) {
    int i;
    for(i = 0; i < block->nodes-1; i++) {
        g_line(block->coords[i].xy[0],block->coords[i+1].xy[0],block->coords[i].xy[1],block->coords[i+1].xy[1],colour);
    }
}

void g_draw_way(way_prop * way, uint8_t colour, uint8_t layer) {
    
    if(way->tag_ids[0] == layer) {
        for(uint8_t way_data = 0; way_data < way->blocks; way_data++) {
            for(uint8_t way_block = 0; way_block < way->data[way_data].polygons; way_block++) {
                //uint8_t poly_type = 0;
                //if(poly_type) {
                //    g_poly(way->data[way_block].block, colour);
                //    g_poly_fill(way->data[way_block].block, colour);
                //} else {
                    g_poly_line(way->data[way_block].block, colour);
                //}
            }
        }
    }
}
