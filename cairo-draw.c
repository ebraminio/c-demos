// gcc cairo-draw.c `pkg-config --libs --cflags cairo` -Wall && ./a.outs
#include <stdio.h>
#include <stdlib.h>
#include <cairo.h>

int main() {
  cairo_surface_t *cairo_surface =
      cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 200, 150);
  cairo_t *cr = cairo_create(cairo_surface);

  cairo_set_source_rgb(cr, .3, .7, .2);
  cairo_set_line_width(cr, 10);
  cairo_move_to(cr, 20, 20);
  cairo_line_to(cr, 20, 130);
  cairo_line_to(cr, 180, 20);
  cairo_stroke(cr);

  cairo_destroy(cr);
  cairo_surface_write_to_png(cairo_surface, "out.png");
  cairo_surface_destroy(cairo_surface);
}
