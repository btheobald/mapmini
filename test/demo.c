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
#include "thick.h"
//#include "agg_hal.h"

#include "map.h"

typedef struct { // Storage for subtile quadrants
    uint16_t subtile_q[4];
} subtile_q_maps;

subtile_q_maps get_st(int16_t xo, int16_t yo, int16_t size) {
    subtile_q_maps tmp = {{0,0,0,0}};

    // Find center
    int8_t xc = (4-4*xo/size);
    int8_t yc = (4-4*yo/size);

    // View extents (Subtile Bounds)
    int8_t xmax = xc+4;
    int8_t xmin = xc-4;
    int8_t ymax = yc+4;
    int8_t ymin = yc-4;

    // Build a binary supertile map (2x2)
    uint8_t map[8][8] = {0};
    for(int y = 0; y < 8; y++) {
        for(int x = 0; x < 8; x++) {
            if(x >= xmin && x < xmax && y >= ymin && y < ymax)
                map[y][x] = 1;
            //printf("%d",map[y][x]);
        }
        //printf("\n");
    }

    // Calculate quadrant subtile words
    int8_t qlxmin, qlxmax;
    int8_t qlymin, qlymax;
    for(int q = 0; q < 4; q++) {
        switch(q) {
            case 0: qlxmin = 0; qlxmax = 3; qlymin = 0; qlymax = 3; break;
            case 1: qlxmin = 4; qlxmax = 7; qlymin = 0; qlymax = 3; break;
            case 2: qlxmin = 0; qlxmax = 3; qlymin = 4; qlymax = 7; break;
            case 3: qlxmin = 4; qlxmax = 7; qlymin = 4; qlymax = 7; break;
        }
        for(int y = 0; y < 4; y++) {
            for(int x = 0; x < 4; x++) {
                tmp.subtile_q[q] |= (map[qlymin+y][qlxmin+x] << (3-x)) << (3-y)*4;
            }
        }
    }
    return tmp;
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

    float xo = 0;
    float yo = 0;
    
    float rot = 0.0;

    int tile_size = 256;

    printf("Arena:\n");

    while (!quit) {

        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    quit = true;
                }
                
                int8_t xc = 0;
                int8_t yc = 0;
                if (event.key.keysym.sym == SDLK_a) {
                    hagl_clear_screen();
                    xc = 8;
                } else if (event.key.keysym.sym == SDLK_d) {
                    hagl_clear_screen();
                    xc = -8;
                }
                
                if (event.key.keysym.sym == SDLK_w) {
                    hagl_clear_screen();
                    yc = 8;
                } else if (event.key.keysym.sym == SDLK_s) {
                    hagl_clear_screen();
                    yc = -8;
                } 
                
                xo+=floor(xc*cos(-rot)-yc*sin(-rot));
                yo+=floor(xc*sin(-rot)+yc*cos(-rot));
                
                if (event.key.keysym.sym == SDLK_q) {
                    hagl_clear_screen();
                    rot+=M_PI/100;
                } else if (event.key.keysym.sym == SDLK_e) {
                    hagl_clear_screen();
                    rot-=M_PI/100;
                } else if (event.key.keysym.sym == SDLK_r) {
                    hagl_clear_screen();
                    rot=0;
                }
                if(rot > M_PI)
                  rot = -M_PI;
                else if(rot < -M_PI)
                  rot = M_PI;

                if (event.key.keysym.sym == SDLK_t) {
                    hagl_clear_screen();
                    tile_size+=5;
                } else if (event.key.keysym.sym == SDLK_g) {
                    hagl_clear_screen();
                    tile_size-=5;
                } else if (event.key.keysym.sym == SDLK_b) {
                    hagl_clear_screen();
                    tile_size=256;
                }
                if(tile_size < 64)
                  tile_size = 64;
                else if(tile_size > 1024)
                  tile_size = 1024;

                draw_varthick_line(DISPLAY_WIDTH/2,DISPLAY_HEIGHT/2,DISPLAY_WIDTH/2+32*cos(rot),DISPLAY_HEIGHT/2+32*sin(rot), 2, hagl_color(255,0,0));
                draw_varthick_line(DISPLAY_WIDTH/2,DISPLAY_HEIGHT/2,DISPLAY_WIDTH/2-32*sin(rot),DISPLAY_HEIGHT/2+32*cos(rot), 2, hagl_color(0,255,0));
                draw_varthick_line(DISPLAY_WIDTH/2,DISPLAY_HEIGHT/2,DISPLAY_WIDTH/2,            DISPLAY_HEIGHT/2-32,          2, hagl_color(0,100,255));

                if(yo < -tile_size/2)      { y_in++; yo =  tile_size/2-8; }
                else if(yo > tile_size/2)  { y_in--; yo = -tile_size/2+8; }
                if(xo < -tile_size/2)      { x_in++; xo =  tile_size/2-8; }
                else if(xo > tile_size/2)  { x_in--; xo = -tile_size/2+8; }

                subtile_q_maps st = get_st(xo,yo,tile_size);

                uint16_t heap = 0;

                if(st.subtile_q[0] != 0x0000) {
                    //printf("Q0: ");
                    heap += load_map("scotland_roads.map", x_in,   y_in,    z_in, (int32_t)xo-tile_size, (int32_t)yo-tile_size, st.subtile_q[0], rot, tile_size);
                }
                if(st.subtile_q[1] != 0x0000) {
                    //printf("Q1: ");
                    heap += load_map("scotland_roads.map", x_in+1, y_in,    z_in, (int32_t)xo,           (int32_t)yo-tile_size, st.subtile_q[1], rot, tile_size);
                }
                if(st.subtile_q[2] != 0x0000) {
                    //printf("Q2: ");
                    heap += load_map("scotland_roads.map", x_in,   y_in+1,  z_in, (int32_t)xo-tile_size, (int32_t)yo, st.subtile_q[2], rot, tile_size);
                }
                if(st.subtile_q[3] != 0x0000) {
                    //printf("Q3: ");
                    heap += load_map("scotland_roads.map", x_in+1, y_in+1,  z_in, (int32_t)xo,           (int32_t)yo, st.subtile_q[3], rot, tile_size);
                }
                printf("%d\n", heap);

                //printf("%f, %f, %f\n", xo, yo, rot);

                //draw_varthick_line((int32_t)xo%tile_size, (int32_t)yo%tile_size+tile_size, (int32_t)xo%tile_size+DISPLAY_WIDTH, (int32_t)yo%tile_size+tile_size, 2, hagl_hal_color(0,255,255));
                //draw_varthick_line((int32_t)xo%tile_size+DISPLAY_WIDTH/2, (int32_t)yo%tile_size, (int32_t)xo%tile_size+DISPLAY_WIDTH/2, (int32_t)yo%tile_size+DISPLAY_HEIGHT, 2, hagl_hal_color(0,255,255));
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