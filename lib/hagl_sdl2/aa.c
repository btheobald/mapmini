#include "aa.h"
#include <math.h>

void dla_changebrightness(color_t from, color_t * to, float br) {
    *to = 0;
    *to |= (((uint8_t)(br * ((from & 0xE0) >> 5))) << 5) & 0xE0; // Red
    *to |= (((uint8_t)(br * ((from & 0x1C) >> 2))) << 2) & 0x1C; // Green
    *to |= (((uint8_t)(br * ((from & 0x03)     )))     ) & 0x03; // Blue
}
 
void dla_plot(int x, int y, color_t col, float br)
{
  color_t oc;
  dla_changebrightness(col, &oc, br);
  hagl_put_pixel(x, y, oc);
}
 
#define ipart_(X) ((int)(X))
#define round_(X) ((int)(((double)(X))+0.5))
#define fpart_(X) (((double)(X))-(double)ipart_(X))
#define rfpart_(X) (1.0-fpart_(X))
 
#define swap_(a, b) do{ __typeof__(a) tmp;  tmp = a; a = b; b = tmp; }while(0);

void draw_line_antialias(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, color_t col) {
  double dx = (double)x2 - (double)x1;
  double dy = (double)y2 - (double)y1;
  if ( fabs(dx) > fabs(dy) ) {
    if ( x2 < x1 ) {
      swap_(x1, x2);
      swap_(y1, y2);
    }
    double gradient = dy / dx;
    double xend = round_(x1);
    double yend = y1 + gradient*(xend - x1);
    double xgap = rfpart_(x1 + 0.5);
    int xpxl1 = xend;
    int ypxl1 = ipart_(yend);
    dla_plot(xpxl1, ypxl1, col, rfpart_(yend)*xgap);
    dla_plot(xpxl1, ypxl1+1, col, fpart_(yend)*xgap);
    double intery = yend + gradient;
 
    xend = round_(x2);
    yend = y2 + gradient*(xend - x2);
    xgap = fpart_(x2+0.5);
    int xpxl2 = xend;
    int ypxl2 = ipart_(yend);
    dla_plot(xpxl2, ypxl2, col, rfpart_(yend) * xgap);
    dla_plot(xpxl2, ypxl2 + 1, col, fpart_(yend) * xgap);
 
    int x;
    for(x=xpxl1+1; x < xpxl2; x++) {
      dla_plot(x, ipart_(intery), col, rfpart_(intery));
      dla_plot(x, ipart_(intery) + 1, col, fpart_(intery));
      intery += gradient;
    }
  } else {
    if ( y2 < y1 ) {
      swap_(x1, x2);
      swap_(y1, y2);
    }
    double gradient = dx / dy;
    double yend = round_(y1);
    double xend = x1 + gradient*(yend - y1);
    double ygap = rfpart_(y1 + 0.5);
    int ypxl1 = yend;
    int xpxl1 = ipart_(xend);
    dla_plot(xpxl1, ypxl1, col, rfpart_(xend)*ygap);
    dla_plot(xpxl1 + 1, ypxl1, col, fpart_(xend)*ygap);
    double interx = xend + gradient;
 
    yend = round_(y2);
    xend = x2 + gradient*(yend - y2);
    ygap = fpart_(y2+0.5);
    int ypxl2 = yend;
    int xpxl2 = ipart_(xend);
    dla_plot(xpxl2, ypxl2, col, rfpart_(xend) * ygap);
    dla_plot(xpxl2 + 1, ypxl2, col, fpart_(xend) * ygap);
 
    int y;
    for(y=ypxl1+1; y < ypxl2; y++) {
      dla_plot(ipart_(interx), y, col, rfpart_(interx));
      dla_plot(ipart_(interx) + 1, y, col, fpart_(interx));
      interx += gradient;
    }
  }
}
#undef swap_
#undef plot_
#undef ipart_
#undef fpart_
#undef round_
#undef rfpart_