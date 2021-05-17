// gcc pangocairo-drawtext.c `pkg-config --libs --cflags pangocairo` -Wall && ./a.out
#include <pango/pangocairo.h>

int main(int argc, const char **argv) {
  int width, height;
  const char *text = argc < 2 ? "سلام" : argv[1];
  PangoFontDescription *font_description = pango_font_description_from_string("Noto Naskh Arabic 600");
  {
    cairo_surface_t *cairo_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 0, 0);
    cairo_t *cr = cairo_create(cairo_surface);
    PangoLayout *pango_layout = pango_cairo_create_layout(cr);
    pango_layout_set_font_description(pango_layout, font_description);
    pango_layout_set_text(pango_layout, text, -1);
    pango_layout_set_auto_dir(pango_layout, 1);
    pango_layout_get_pixel_size(pango_layout, &width, &height);
    cairo_destroy(cr);
    cairo_surface_destroy(cairo_surface);
  }
  {
    cairo_surface_t *cairo_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t *cr = cairo_create(cairo_surface);
    PangoLayout *pango_layout = pango_cairo_create_layout(cr);
    pango_layout_set_font_description(pango_layout, font_description);
    pango_layout_set_text(pango_layout, text, -1);
    pango_layout_set_auto_dir(pango_layout, 1);
    cairo_set_source_rgba(cr, 0, 0, 0, 0);
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 1, 1, 1);
    pango_cairo_show_layout(cr, pango_layout);
    cairo_surface_write_to_png(cairo_surface, "out.png");
    cairo_surface_destroy(cairo_surface);
    cairo_destroy(cr);
  }
  pango_font_description_free(font_description);
  return 0;
}
