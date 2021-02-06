// gcc harfbuzz-shape.c `pkg-config --libs --cflags harfbuzz` -Wall && a.out
#include <stdio.h>
#include <stdlib.h>

#include <hb.h>

int main() {
  hb_blob_t *blob = hb_blob_create_from_file("input/Roboto.abc.ttf");
  // or read the file yourself into buf and use,
  //   hb_blob_create(buf, len, HB_MEMORY_MODE_WRITABLE, buf, free);
  hb_face_t *face = hb_face_create(blob, 0);
  hb_blob_destroy(blob);
  hb_font_t *font = hb_font_create(face);
  hb_face_destroy(face);

  // consider 36 as font size and 64 as subpixel percision
  hb_font_set_scale(font, 36 * 64, 36 * 64);

  hb_buffer_t *buffer = hb_buffer_create();
  hb_buffer_add_utf8(buffer, "abc", -1, 0, -1);
  hb_buffer_guess_segment_properties(buffer);
  // or use these instead guess:
  //   hb_buffer_set_direction(buffer, HB_DIRECTION_LTR);
  //   hb_buffer_set_script(buffer, HB_SCRIPT_LATIN);
  hb_shape(font, buffer, NULL, 0);

  unsigned len = hb_buffer_get_length(buffer);
  hb_glyph_info_t *info = hb_buffer_get_glyph_infos(buffer, NULL);
  hb_glyph_position_t *pos = hb_buffer_get_glyph_positions(buffer, NULL);

  double current_x = 0;
  double current_y = 0;
  for (unsigned int i = 0; i < len; i++) {
    char glyph_name[32];
    hb_font_get_glyph_name(font, info[i].codepoint, glyph_name,
                           sizeof(glyph_name));
    double x_position = current_x + pos[i].x_offset / 64.;
    double y_position = current_y + pos[i].y_offset / 64.;
    printf("gid: %s (%d), position: (%g, %g), str index: %d\n", glyph_name,
           info[i].codepoint, x_position, y_position, info[i].cluster);
    current_x += pos[i].x_advance / 64.;
    current_y += pos[i].y_advance / 64.;
  }

  hb_font_destroy(font);

  return 0;
}
