#include "agg_hal.h"
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <string.h>
#include "agg_pixfmt_rgba.h"
#include "agg_renderer_base.h"
#include "agg_conv_stroke.h"
#include "agg_conv_curve.h"
#include "agg_conv_dash.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_renderer_scanline.h"
#include "agg_scanline_p.h"
#include "agg_ellipse.h"
#include "agg_rounded_rect.h"
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

 void drawLine (agg::renderer_base<agg::pixfmt_rgba32>& rBase, double x1, double y1, double x2, double y2, double lineWidth, agg::rgba8 color) {
     agg::rasterizer_scanline_aa<> ras;
     agg::scanline_p8 scanline;
 
     ras.auto_close(false);
 
     double linePoints[] = { x1, y1, x2, y2 };
     SimplePath path (linePoints, sizeof(linePoints) / sizeof(double));
     agg::conv_stroke<SimplePath> strokePath (path);
 
     strokePath.width(lineWidth);
     strokePath.line_cap(agg::butt_cap);
     strokePath.line_join(agg::miter_join);
     strokePath.miter_limit(lineWidth);
 
     ras.reset();
     ras.add_path(strokePath);
 
     agg::render_scanlines_aa_solid(ras, scanline, rBase, color);
 }

void agg_hal_test(void) {
     const agg::rgba8 transparentWhiteColor (0xff, 0xff, 0xff, 0);
     const agg::rgba8 redColor (0xff, 0, 0, 0xff);
     const agg::rgba8 pinkColor (0xff, 0, 0xff, 0xff);
     const agg::rgba8 greenColor (0, 0xff, 0, 0xff);
     const agg::rgba8 cyanColor (0, 0xff, 0xff, 0xff);
     const agg::rgba8 blueColor (0, 0, 0xff, 0xff);
     const agg::rgba8 yellowColor (0xff, 0xff, 0x0, 0xff);
     const agg::rgba8 blackColor (0, 0, 0, 0xff);
     const agg::rgba8 opaqueRedColor (0xff, 0x00, 0x00, 0x44);

     // clear the buffer with transparent white color
     rBase.clear(transparentWhiteColor);

     agg::rasterizer_scanline_aa<> ras;
     agg::scanline_p8 scanline;
 
     ras.auto_close(true);

     // draw a green border
     {
         double border[] =
         {
             25, 25,
             DISPLAY_WIDTH/2, 25,
             DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2,
             25, DISPLAY_HEIGHT/2,
             25, 25
         };

         SimplePath path (border, sizeof(border) / sizeof(double));
         agg::conv_stroke<SimplePath> strokePath (path);

         double strokeWidth = 4.0;
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

         double strokeWidth = 6.0;
         strokePath.width(strokeWidth);
         strokePath.line_cap(agg::round_cap);
         strokePath.line_join(agg::round_join);
         strokePath.miter_limit(strokeWidth);

         ras.reset();
         ras.add_path(strokePath);

         agg::render_scanlines_aa_solid(ras, scanline, rBase, redColor);
     }
     {
       // draw a square with a square hole
       CmdVertex squares[] =
       {
          // outer square is CW
          { agg::path_cmd_move_to, 100, 110 },
          { agg::path_cmd_line_to, 160, 110 },
          { agg::path_cmd_line_to, 160, 190 },
          { agg::path_cmd_line_to, 100, 190 },
          { agg::path_cmd_line_to, 100, 110 },

          // inner square is also CW
          { agg::path_cmd_move_to, 100, 130 },
          { agg::path_cmd_line_to, 140, 130 },
          { agg::path_cmd_line_to, 140, 170 },
          { agg::path_cmd_line_to, 100, 170 },
          { agg::path_cmd_line_to, 100, 130 },
       };

       drawLine (rBase, 0, 5, 100, 5, 0.5, blackColor);
       drawLine (rBase, 0, 8.1, 100, 8.1, 0.5, blackColor);
       drawLine (rBase, 0, 11.3, 100, 11.3, 0.5, blackColor);
       drawLine (rBase, 0, 14.5, 100, 14.5, 0.5, blackColor);
       drawLine (rBase, 0, 17.7, 100, 17.7, 0.5, blackColor);
     

         
         CmdVertex tmp = squares[6];
         squares[6] = squares[8];
         squares[8] = tmp;

         CmdVertexPath path (squares, sizeof(squares) / sizeof(CmdVertex));

       ras.reset();
       ras.add_path(path);

       agg::render_scanlines_aa_solid(ras, scanline, rBase, blueColor);

     }
     {

        CmdVertex dome[] =
         {
            { agg::path_cmd_move_to,  20, 200 },
            { agg::path_cmd_curve3,  150, 300 },
            { agg::path_cmd_curve3,  200, 100 },
         };
 
         CmdVertexPath path (dome, sizeof(dome) / sizeof(CmdVertex));
 
         // draw a dashed dot caret
         {
             double strokeWidth = 5.0;
 
             agg::conv_dash<CmdVertexPath> dashPath (path);
 
             // a long dash
             dashPath.add_dash(3 * strokeWidth, 3 * strokeWidth);
             // a dot
             dashPath.add_dash(1 * strokeWidth, 3 * strokeWidth);
 
             agg::conv_stroke<agg::conv_dash<CmdVertexPath> > strokePath (dashPath);
 
             strokePath.width(strokeWidth);
             strokePath.line_cap(agg::butt_cap);
             strokePath.line_join(agg::miter_join);
             strokePath.miter_limit(strokeWidth);
 
             ras.reset();
             ras.add_path(strokePath);
 
             agg::render_scanlines_aa_solid(ras, scanline, rBase, yellowColor);
         }
 
         // draw a dashed dot dome
         {
             double strokeWidth = 5.0;
 
             // generate a curved path
             agg::conv_curve<CmdVertexPath> curvePath (path);
 
             // and then use this curved path to generate the stroke path
             agg::conv_stroke<agg::conv_curve<CmdVertexPath> > strokePath (curvePath);
 
             strokePath.width(strokeWidth);
             strokePath.line_cap(agg::square_cap);
             strokePath.line_join(agg::miter_join);
             strokePath.miter_limit(strokeWidth);
 
             ras.reset();
             ras.add_path(strokePath);
 
             agg::render_scanlines_aa_solid(ras, scanline, rBase, pinkColor);
         }
         
         {
             agg::ellipse eliipse (200, 70, 20, 50);
             agg::rounded_rect rect2 (10, 100, 110, 320, 10);
             
             ras.reset();
             ras.add_path(rect2);
             agg::render_scanlines_aa_solid(ras, scanline, rBase, opaqueRedColor);
 
             ras.reset();
             ras.add_path(eliipse);
 
             agg::render_scanlines_aa_solid(ras, scanline, rBase, blackColor);
         }
     }

     agg_hal_flush();
}
