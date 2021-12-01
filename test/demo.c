/*

MIT No Attribution

Copyright (c) 2019-2021 Mika Tuupola

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

-cut-

SPDX-License-Identifier: MIT-0

*/
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "hagl_hal.h"
#include "hagl.h"
#include "rgb565.h"
#include "fps.h"
#include "font6x9.h"

//#include "agg_hal.h"

#include "map.h"

uint16_t get_st(int16_t xo, int16_t yo, uint8_t q) {
    int8_t xc = 4-(xo/64);
    int8_t yc = 4-(yo/64);

    // View extents
    int8_t xmax = xc+2;
    int8_t xmin = xc-2;
    int8_t ymax = yc+2;
    int8_t ymin = yc-2;

    uint8_t map[8][8] = {0};
    for(int y = 0; y < 8; y++) {
        for(int x = 0; x < 8; x++) {
            if(x >= xmin && x < xmax && y >= ymin && y < ymax)
                map[y][x] = 1;
            //printf("%d", map[y][x]);
        }
        //printf("\n");
    }

    int8_t qlxmin, qlxmax;
    int8_t qlymin, qlymax;
    switch(q) {
        case 0: qlxmin = 0; qlxmax = 3; qlymin = 0; qlymax = 3; break;
        case 1: qlxmin = 4; qlxmax = 7; qlymin = 0; qlymax = 3; break;
        case 2: qlxmin = 0; qlxmax = 3; qlymin = 4; qlymax = 7; break;
        case 3: qlxmin = 4; qlxmax = 7; qlymin = 4; qlymax = 7; break;
    }

    uint16_t st_map = 0;
    for(int y = 0; y <= 3; y++) {
        for(int x = 0; x <= 3; x++) {
            st_map |= (map[qlymin+y][qlxmin+x] << (3-x)) << (3-y)*4;
            //printf("%d", (map[qlymin+y][qlxmin+x]));
        }
        //printf("\n");
    }

    return st_map;
}

int main(int argc, char *argv[]) {
    hagl_init();
    hagl_set_clip_window(1,1,DISPLAY_WIDTH-1,DISPLAY_HEIGHT-1);
    //hagl_set_clip_window(DISPLAY_WIDTH/4,DISPLAY_HEIGHT/4,DISPLAY_WIDTH-DISPLAY_WIDTH/4,DISPLAY_HEIGHT-DISPLAY_HEIGHT/4);
    //agg_hal_init();
    //agg_hal_test();
    //agg_hal_flush();
    srand(time(0));

    uint32_t flush_delay = 1000 / 30; /* 30 fps */
    uint32_t pps_delay = 2000; /* 0.5 fps */
    uint16_t current_demo = 0;
    float current_pps = 0.0; /* primitives per secod */

    bool quit = false;
    SDL_Event event;

    printf("\n");
    printf("Press any key to change demo.\n");
    printf("Press ESC to quit.\n\n");

    hagl_clear_screen();
    uint32_t start = SDL_GetTicks();

    uint32_t x_in = 8044;
    uint32_t y_in = 5108;
    uint32_t z_in = 14;

    int16_t xo = 0;
    int16_t yo = 0;

    const int tile_size = 256;

    while (!quit) {

        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    quit = true;
                } else if (event.key.keysym.sym == SDLK_LEFT) {
                    hagl_clear_screen();
                    xo+=8;
                } else if (event.key.keysym.sym == SDLK_RIGHT) {
                    hagl_clear_screen();
                    xo-=8;
                } else if (event.key.keysym.sym == SDLK_UP) {
                    hagl_clear_screen();
                    yo+=8;
                } else if (event.key.keysym.sym == SDLK_DOWN) {
                    hagl_clear_screen();
                    yo-=8;
                } else {                    
                    current_demo = (current_demo + 1) % 12;        
                }

                if(yo < -tile_size/2)      { y_in++; yo =  tile_size/2-8; }
                else if(yo > tile_size/2) { y_in--; yo = -tile_size/2+8; }
                if(xo < -tile_size/2)      { x_in++; xo =  tile_size/2-8; }
                else if(xo > tile_size/2) { x_in--; xo = -tile_size/2+8; }

                printf("Q0: ");
                load_map("scotland_roads.map", x_in,   y_in,    z_in, xo%tile_size,           yo%tile_size, get_st(xo,yo,0));
                printf("Q1: ");
                load_map("scotland_roads.map", x_in+1, y_in,    z_in, xo%tile_size+tile_size, yo%tile_size, get_st(xo,yo,1));
                printf("Q2: ");
                load_map("scotland_roads.map", x_in,   y_in+1,  z_in, xo%tile_size,           yo%tile_size+tile_size, get_st(xo,yo,2));
                printf("Q3: ");
                load_map("scotland_roads.map", x_in+1, y_in+1,  z_in, xo%tile_size+tile_size, yo%tile_size+tile_size, get_st(xo,yo,3));
                printf("\n");

                printf("%d, %d\n", xo, yo);

                draw_varthick_line(xo%tile_size, yo%tile_size+tile_size, xo%tile_size+DISPLAY_WIDTH, yo%tile_size+tile_size, 2, hagl_hal_color(0,255,255));
                draw_varthick_line(xo%tile_size+DISPLAY_WIDTH/2, yo%tile_size, xo%tile_size+DISPLAY_WIDTH/2, yo%tile_size+DISPLAY_HEIGHT, 2, hagl_hal_color(0,255,255));
            }
        } 

    //if((SDL_GetTicks() - start) >= 16) { // Ludicrious Mode
    //    start = SDL_GetTicks();
        hagl_hal_flush();
    //}

    }

    //agg_hal_close();
    return 0;
}