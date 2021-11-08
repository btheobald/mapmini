#include "agg_hal.h"
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <string.h>
#include "agg_pixfmt_rgba.h"
#include "agg_renderer_base.h"
#include "agg_conv_stroke.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_renderer_scanline.h"
#include "agg_scanline_p.h"
#include "path.h"

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;

agg::rendering_buffer rbuf;
agg::pixfmt_rgba32  pixFmt;
agg::renderer_base<agg::pixfmt_rgba32>  rBase;

unsigned char buffer[DISPLAY_WIDTH*DISPLAY_HEIGHT*(const int)agg::pixfmt_rgba32::pix_width];

void agg_hal_init(void)
{   
    rbuf.attach(buffer, DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_WIDTH*(const int)agg::pixfmt_rgba32::pix_width);
    pixFmt.attach(rbuf);
    rBase.attach(pixFmt);

    printf("%d %d %d\n", DISPLAY_WIDTH, DISPLAY_HEIGHT, (const int)agg::pixfmt_rgba32::pix_width);

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
    window = SDL_CreateWindow(
        "AGG SDL2",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        DISPLAY_WIDTH*DISPLAY_SCALE,
        DISPLAY_HEIGHT*DISPLAY_SCALE,
        0
    );

    if (NULL == window) {
        printf("Could not create window: %s\n", SDL_GetError());
    }

    renderer = SDL_CreateRenderer(window, -1, 0);

    SDL_RenderSetScale(renderer, DISPLAY_SCALE, DISPLAY_SCALE);

    if (NULL == renderer) {
        printf("Could not create renderer: %s\n", SDL_GetError());
    }

    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STATIC,
        DISPLAY_WIDTH,
        DISPLAY_HEIGHT
    );

    if (NULL == texture) {
        printf("Could not create texture: %s\n", SDL_GetError());
    }

    SDL_PumpEvents();
}

/*
 * Flushes the back buffer to the SDL2 window
 */
void agg_hal_flush()
{
    SDL_UpdateTexture(texture, NULL, buffer, DISPLAY_WIDTH*4);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void agg_hal_close(void)
{
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void agg_hal_test(void) {
     const agg::rgba8 transparentWhiteColor (0xff, 0xff, 0xff, 0);
     const agg::rgba8 greenColor (0, 0xff, 0, 0xff);
     const agg::rgba8 redColor (0xff, 0, 0, 0xff);

     // clear the buffer with transparent white color
     rBase.clear(transparentWhiteColor);

     agg::rasterizer_scanline_aa<> ras;
     agg::scanline_p8 scanline;
 
     ras.auto_close(true);

     // draw a green border
     {
         double border[] =
         {
             0, 0,
             DISPLAY_WIDTH/2, 0,
             DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2,
             0, DISPLAY_HEIGHT/2,
             0, 0
         };

         SimplePath path (border, sizeof(border) / sizeof(double));
         agg::conv_stroke<SimplePath> strokePath (path);

         double strokeWidth = 1.0;
         strokePath.width(strokeWidth);
         strokePath.line_cap(agg::square_cap);
         strokePath.line_join(agg::miter_join);
         strokePath.miter_limit(strokeWidth);

         ras.reset();
         ras.add_path(strokePath);

         agg::render_scanlines_aa_solid(ras, scanline, rBase, greenColor);
     }

     // draw a red star
     {
         double star[] =
         {
              50,  3,
              20, 97,
              95, 37,
               5, 37,
              80, 97,
              50,  3
         };

         SimplePath path (star, sizeof(star) / sizeof(double));

         agg::conv_stroke<SimplePath> strokePath (path);

         double strokeWidth = 2.0;
         strokePath.width(strokeWidth);
         strokePath.line_cap(agg::square_cap);
         strokePath.line_join(agg::miter_join);
         strokePath.miter_limit(strokeWidth);

         ras.reset();
         ras.add_path(strokePath);

         agg::render_scanlines_aa_solid(ras, scanline, rBase, redColor);
     }

     agg_hal_flush();
}
