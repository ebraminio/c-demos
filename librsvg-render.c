// gcc librsvg-render.c `pkg-config --libs --cflags librsvg-2.0` -Wall && ./a.out
#include <librsvg/rsvg.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  uint8_t *buf;
  unsigned len;
  {
    FILE *f = fopen("input/example.svg", "rb");
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    rewind(f);
    buf = (uint8_t *)malloc(len);
    len = fread(buf, 1, len, f);
    fclose(f);
  }

  GError *error = NULL;
  RsvgHandle *handle = rsvg_handle_new_from_data(buf, len, &error);
  if (error)
    return 1;
  RsvgDimensionData dim;
  rsvg_handle_get_dimensions(handle, &dim);

  cairo_surface_t *cairo_surface =
      cairo_image_surface_create(CAIRO_FORMAT_ARGB32, dim.width, dim.height);
  cairo_t *cr = cairo_create(cairo_surface);
  rsvg_handle_render_cairo(handle, cr);
  cairo_destroy(cr);
  cairo_surface_write_to_png(cairo_surface, "out.png");
  cairo_surface_destroy(cairo_surface);
}
