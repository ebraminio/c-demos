
#include <cairo.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  unsigned width = 200, height = 150;
  uint8_t *image = calloc(height, width * 4);
  {
    /* the recommended way is to use cairo_format_stride_for_width */
    unsigned stride = width * 4;
    cairo_surface_t *cairo_surface = cairo_image_surface_create_for_data(
        image, CAIRO_FORMAT_ARGB32, width, height, stride);
    cairo_t *cr = cairo_create(cairo_surface);

    cairo_set_source_rgb(cr, .3, .7, .2);
    cairo_set_line_width(cr, 10);
    cairo_move_to(cr, 20, 20);
    cairo_line_to(cr, 20, 130);
    cairo_line_to(cr, 180, 20);
    cairo_stroke(cr);

    cairo_destroy(cr);
    cairo_surface_destroy(cairo_surface);
  }

  {
    // Just that .pbm doesn't supoprt 4 channels let's turn it into 3 channels
    // https://stackoverflow.com/a/18495447
    for (int i = 0; i < width * height; i++) {
      int r = image[i * 4 + 0], g = image[i * 4 + 1], b = image[i * 4 + 2],
          a = image[i * 4 + 3];

      image[i * 3 + 0] = r * a / 255;
      image[i * 3 + 1] = g * a / 255;
      image[i * 3 + 2] = b * a / 255;
    }
  }
  {
    FILE *f = fopen("out.pbm", "wb");
    fprintf(f, "P6 %d %d 255\n", width, height);
    fwrite(image, sizeof(char), width * height * 4, f);
    fclose(f);
  }
  free(image);
}