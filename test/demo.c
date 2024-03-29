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
#include <unistd.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "hagl_hal.h"
#include "hagl.h"
#include "rgb332.h"
#include "fps.h"
#include "font6x9.h"
#include "thick.h"
#include "aa.h"
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
    int8_t xmax = xc+1024/size;
    int8_t xmin = xc-1024/size;
    int8_t ymax = yc+1024/size;
    int8_t ymin = yc-1024/size;

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

    srand(time(0));

    uint32_t flush_delay = 1000 / 30; /* 30 fps */
    uint32_t pps_delay = 2000; /* 0.5 fps */
    uint16_t current_demo = 0;
    float current_pps = 0.0; /* primitives per secod */

    bool quit = false;

    printf("\n");
    printf("Press any key to change demo.\n");
    printf("Press ESC to quit.\n\n");

    hagl_clear_screen();
    uint32_t start = SDL_GetTicks();

    float rot = 0.0;
    int update = false;
    
    fb_handler fbh;
    load_map_file(&fbh, "scotland_roads.map");

    mm_file_header_t mm_header;
    load_map_header(&fbh, &mm_header);

    arena_t mm_arena;
    arena_init(&mm_arena, ARENA_DEFAULT_SIZE);

    mm_view_params_t mm_view = {.x_offset = 0, .y_offset = 0, .rotation_cos = 0, .rotation_sin = 0, .subtile = 0xffff, .scale = 512};
    mm_tile_coord_t mm_xyz = {.x = 8044, .y = 5108, .z = 14};

    mm_tile_header mm_tile;

    update = 1;

    SDL_Event event;

    while (!quit) {
        
        int8_t xc = 0;
        int8_t yc = 0;

        while(SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
            if (event.type == SDL_KEYDOWN) {
                update = true;

                xc = 0;
                yc = 0;

                // Process keypress
                switch(event.key.keysym.sym) {
                    case SDLK_ESCAPE: quit = true; break;
                    case SDLK_a:
                    case SDLK_LEFT: xc = 4; break;
                    case SDLK_d:
                    case SDLK_RIGHT: xc = -4; break;
                    case SDLK_w:
                    case SDLK_UP: yc = 4; break;
                    case SDLK_s:
                    case SDLK_DOWN: yc = -4; break;
                    case SDLK_q: rot += M_PI/314; break;
                    case SDLK_e: rot -= M_PI/314; break;
                    case SDLK_r: rot  = 0; break;
                    case SDLK_t: mm_view.scale += 4; break;
                    case SDLK_g: mm_view.scale -= 4; break;
                    case SDLK_b: mm_view.scale  = 256; break;
                }

                // Check rotation bounds
                if(rot > M_PI)
                    rot = -M_PI;
                else if(rot < -M_PI)
                    rot = M_PI;

                mm_view.rotation_cos = cos(rot)*FIXED_POINT_SCALE;
                mm_view.rotation_sin = sin(rot)*FIXED_POINT_SCALE;

                // Check zoom bounds
                if(mm_view.scale < 64)
                    mm_view.scale = 64;
                else if(mm_view.scale > 1024)
                    mm_view.scale = 1024;

                // Apply transformation and rotation
                mm_view.x_offset+=(xc*mm_view.rotation_cos-yc*mm_view.rotation_sin)/FIXED_POINT_SCALE;
                mm_view.y_offset+=(xc*mm_view.rotation_sin+yc*mm_view.rotation_cos)/FIXED_POINT_SCALE;

                // Check offset bounds, change tiles if needed
                if(mm_view.y_offset < -mm_view.scale/2)      { mm_xyz.y++; mm_view.y_offset = -4+mm_view.scale/2; }
                else if(mm_view.y_offset > mm_view.scale/2)  { mm_xyz.y--; mm_view.y_offset =  4-mm_view.scale/2; }
                if(mm_view.x_offset < -mm_view.scale/2)      { mm_xyz.x++; mm_view.x_offset = -4+mm_view.scale/2; }
                else if(mm_view.x_offset > mm_view.scale/2)  { mm_xyz.x--; mm_view.x_offset =  4-mm_view.scale/2; }
            }
        } 

        if(update) {
            hagl_clear_screen();

            update = false;
            subtile_q_maps st = get_st(mm_view.x_offset, mm_view.y_offset, mm_view.scale);

            uint32_t heap;
            uint32_t heap_total = 0;

            //if(st.subtile_q[0] != 0x0000) {
                //printf("Q0: ");
                //heap = load_map("scotland_roads.map", x_in,   y_in,   z_in, (int32_t)xo-tile_size, (int32_t)yo-tile_size, st.subtile_q[0], rot, tile_size);
                //mm_view.subtile = st.subtile_q[0];

                //printf("Arena Used: %d", heap);
            //}
            /*if(st.subtile_q[1] != 0x0000) {
                //printf("Q1: ");
                heap = load_map("scotland_roads.map", x_in+1, y_in,   z_in, (int32_t)xo,           (int32_t)yo-tile_size, st.subtile_q[1], rot, tile_size);
                heap_total += heap;
                //printf("%d ", heap);
            }
            if(st.subtile_q[2] != 0x0000) {
                //printf("Q2: ");
                heap = load_map("scotland_roads.map", x_in,   y_in+1, z_in, (int32_t)xo-tile_size, (int32_t)yo, st.subtile_q[2], rot, tile_size);
                heap_total += heap;
                //printf("%d ", heap);
            }*/
            /*if(st.subtile_q[3] != 0x0000) {
                mm_view.subtile = st.subtile_q[3];
                heap = load_map_tile(&fbh, &mm_header, &mm_xyz, &mm_view, &mm_arena);
                heap_total += heap;
            }*/

            mm_view.subtile = 0xffff;
            load_map_tile(&fbh, &mm_header, &mm_tile, &mm_xyz, &mm_view, &mm_arena);
            printf("%d, %d, %d\n", mm_xyz.x, mm_xyz.y, mm_xyz.z);
            arena_free(&mm_arena);

            uint16_t compass_len = 32;
            uint16_t border = 3;
            uint16_t compass_x = compass_len+border;
            uint16_t compass_y = DISPLAY_HEIGHT-compass_len-border;
            uint8_t compass_lw = 2;

            hagl_fill_circle(compass_x, compass_y, compass_len+2, hagl_color(0,0,0));
            hagl_draw_circle(compass_x, compass_y, compass_len+2, hagl_color(255,255,255));
            draw_varthick_line(compass_x, compass_y, compass_x-(compass_len-1)*sin(-rot), compass_y-(compass_len-1)*cos(-rot), compass_lw, hagl_color(255,0,0));
            hagl_fill_circle(compass_x, compass_y, 3, hagl_color(255,255,255));
        }


        if((SDL_GetTicks() - start) >= 16) {
            start = SDL_GetTicks();
            hagl_hal_flush();
        } else {
            usleep(1);
        }
    }

    return 0;
}
