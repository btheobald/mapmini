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
#include <SDL2/SDL.h>

#include "hagl_hal.h"
#include "hagl.h"
#include "rgb565.h"
#include "fps.h"
#include "font6x9.h"

#include "agg_hal.h"

#include "map.h"

int main()
{
    //hagl_init();
    agg_hal_init();
    agg_hal_test();
    agg_hal_flush();
    //pod_set_clip_window(0, 30, 319, 210);
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

    //hagl_clear_screen();
    uint32_t start = SDL_GetTicks();

    uint32_t x_in = 8044;
    uint32_t y_in = 5108;
    uint32_t z_in = 14;

    //load_map("scotland_roads.map", x_in, y_in, z_in);

    while (!quit) {

        current_pps = fps();

        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    quit = true;
                } else if (event.key.keysym.sym == SDLK_LEFT) {
                    x_in--;
                    //hagl_clear_screen();
                    //load_map("scotland_roads.map", x_in, y_in, z_in);
                } else if (event.key.keysym.sym == SDLK_RIGHT) {
                    x_in++;
                    //hagl_clear_screen();
                    //load_map("scotland_roads.map", x_in, y_in, z_in);
                } else if (event.key.keysym.sym == SDLK_UP) {
                    y_in--;
                    //hagl_clear_screen();
                    //load_map("scotland_roads.map", x_in, y_in, z_in);
                } else if (event.key.keysym.sym == SDLK_DOWN) {
                    y_in++;
                    //hagl_clear_screen();
                    //load_map("scotland_roads.map", x_in, y_in, z_in);
                } else {
                    //fps_reset();
                    //
                    //current_demo = (current_demo + 1) % 12;        
                }
            }
        } 

    if((SDL_GetTicks() - start) >= 100) { // Ludicrious Mode
        start = SDL_GetTicks();
    }
    //hagl_hal_flush();

    }

    agg_hal_close();
    return 0;
}
