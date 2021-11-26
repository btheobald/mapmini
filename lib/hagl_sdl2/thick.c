#include "thick.h"
#include <math.h>
#include <hagl_hal.h>
#include <hagl.h>

/***********************************************************************
 *                                                                     *
 *                            X BASED LINES                            *
 *                                                                     *
 ***********************************************************************/

static void x_perpendicular(int16_t x0,int16_t y0,int16_t dx,int16_t dy,int16_t xstep, int16_t ystep,int16_t einit,int16_t t,int16_t winit, color_t color)
{
  int16_t x,y,threshold,E_diag,E_square;
  int16_t tk;
  int16_t error;
  int16_t p,q;

  threshold = dx - 2*dy;
  E_diag= -2*dx;
  E_square= 2*dy;
  p=q=0;

  y= y0;
  x= x0;
  error= einit;
  tk= dx+dy-winit; 

  while(tk<=t)
  {
     hagl_put_pixel(x,y, color);
     if (error>=threshold)
     {
       x= x + xstep;
       error = error + E_diag;
       tk= tk + 2*dy;
     }
     error = error + E_square;
     y= y + ystep;
     tk= tk + 2*dx;
     q++;
  }

  y= y0;
  x= x0;
  error= -einit;
  tk= dx+dy+winit;

  while(tk<=t)
  {
     if (p)
       hagl_put_pixel(x,y, color);
     if (error>threshold)
     {
       x= x - xstep;
       error = error + E_diag;
       tk= tk + 2*dy;
     }
     error = error + E_square;
     y= y - ystep;
     tk= tk + 2*dx;
     p++;
  }

  if (q==0 && p<2) hagl_put_pixel(x0,y0,color); // we need this for very thin lines
}


static void x_varthick_line(int16_t x0,int16_t y0,int16_t dx,int16_t dy,int16_t xstep, int16_t ystep, int16_t t, int16_t pxstep,int16_t pystep,color_t color)
{
  int16_t p_error, error, x,y, threshold, E_diag, E_square, length, p;
  double D;


  p_error= 0;
  error= 0;
  y= y0;
  x= x0;
  threshold = dx - 2*dy;
  E_diag= -2*dx;
  E_square= 2*dy;
  length = dx+1;
  D= sqrt(dx*dx+dy*dy);

  for(p=0;p<length;p++)
  {
    x_perpendicular(x,y, dx, dy, pxstep, pystep,p_error,t*2*D,error,color);
    if (error>=threshold)
    {
      y= y + ystep;
      error = error + E_diag;
      if (p_error>=threshold) 
      {
        x_perpendicular(x,y, dx, dy, pxstep, pystep,(p_error+E_diag+E_square),t*2*D,error,color);
        p_error= p_error + E_diag;
      }
      p_error= p_error + E_square;
    }
    error = error + E_square;
    x= x + xstep;
  }
}

/***********************************************************************
 *                                                                     *
 *                            Y BASED LINES                            *
 *                                                                     *
 ***********************************************************************/

static void y_perpendicular(int16_t x0,int16_t y0,int16_t dx,int16_t dy,int16_t xstep, int16_t ystep, int16_t einit,int16_t t,int16_t winit,color_t color)
{
  int16_t x,y,threshold,E_diag,E_square;
  int16_t tk;
  int16_t error;
  int16_t p,q;

  p=q= 0;
  threshold = dy - 2*dx;
  E_diag= -2*dy;
  E_square= 2*dx;

  y= y0;
  x= x0;
  error= -einit;
  tk= dx+dy+winit; 

  while(tk<=t)
  {
     hagl_put_pixel(x,y, color);
     if (error>threshold)
     {
       y= y + ystep;
       error = error + E_diag;
       tk= tk + 2*dx;
     }
     error = error + E_square;
     x= x + xstep;
     tk= tk + 2*dy;
     q++;
  }


  y= y0;
  x= x0;
  error= einit;
  tk= dx+dy-winit; 

  while(tk<=t)
  {
     if (p)
       hagl_put_pixel(x,y, color);
     if (error>=threshold)
     {
       y= y - ystep;
       error = error + E_diag;
       tk= tk + 2*dx;
     }
     error = error + E_square;
     x= x - xstep;
     tk= tk + 2*dy;
     p++;
  }

  if (q==0 && p<2) hagl_put_pixel(x0,y0,color); // we need this for very thin lines
}


static void y_varthick_line(int16_t x0,int16_t y0,int16_t dx,int16_t dy,int16_t xstep, int16_t ystep, int16_t t, int16_t pxstep,int16_t pystep, color_t color) {
  int16_t p_error, error, x,y, threshold, E_diag, E_square, length, p;
  double D;

  p_error= 0;
  error= 0;
  y= y0;
  x= x0;
  threshold = dy - 2*dx;
  E_diag= -2*dy;
  E_square= 2*dx;
  length = dy+1;
  D= sqrt(dx*dx+dy*dy);

  for(p=0;p<length;p++)
  {
    y_perpendicular(x,y, dx, dy, pxstep, pystep,p_error,t*2*D,error,color);
    if (error>=threshold)
    {
      x= x + xstep;
      error = error + E_diag;
      if (p_error>=threshold)
      {
        y_perpendicular(x,y, dx, dy, pxstep, pystep, p_error+E_diag+E_square,t*2*D,error,color);
        p_error= p_error + E_diag;
      }
      p_error= p_error + E_square;
    }
    error = error + E_square;
    y= y + ystep;
  }
}


/***********************************************************************
 *                                                                     *
 *                                ENTRY                                *
 *                                                                     *
 ***********************************************************************/

void draw_varthick_line(int16_t x0,int16_t y0,int16_t x1, int16_t y1, int16_t t, color_t color) {
  int16_t dx,dy,xstep,ystep;
  int16_t pxstep = 0; 
  int16_t pystep = 0;

  dx= x1-x0;
  dy= y1-y0;
  xstep= ystep= 1;

  if (dx<0) { dx= -dx; xstep= -1; }
  if (dy<0) { dy= -dy; ystep= -1; }

  if (dx==0) xstep= 0;
  if (dy==0) ystep= 0;

  switch(xstep + ystep*4)
  {
    case -1 + -1*4 :  pystep= -1; pxstep=  1; break;  // -5
    case -1 +  0*4 :  pystep= -1; pxstep=  0; break;  // -1
    case -1 +  1*4 :  pystep=  1; pxstep=  1; break;  // 3
    case  0 + -1*4 :  pystep=  0; pxstep= -1; break;  // -4
    case  0 +  0*4 :  pystep=  0; pxstep=  0; break;  // 0
    case  0 +  1*4 :  pystep=  0; pxstep=  1; break;  // 4
    case  1 + -1*4 :  pystep= -1; pxstep= -1; break;  // -3
    case  1 +  0*4 :  pystep= -1; pxstep=  0; break;  // 1
    case  1 +  1*4 :  pystep=  1; pxstep= -1; break;  // 5
  }

  if (dx>dy) x_varthick_line(x0,y0,dx,dy,xstep,ystep,t,pxstep,pystep,color);
        else y_varthick_line(x0,y0,dx,dy,xstep,ystep,t,pxstep,pystep,color);
}