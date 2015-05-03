// Cairo free version of https://github.com/behdad/harfbuzz-tutorial/blob/master/hello-harfbuzz.c
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <hb.h>
#include <hb-ft.h>

#ifdef PNG_OUTPUT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#endif

#define FONT_SIZE 36
#define MARGIN (FONT_SIZE * .5)

int
main(int argc, char **argv)
{
  const char *fontfile;
  const char *text;

  if (argc < 3)
  {
    fprintf (stderr, "usage: hello-harfbuzz font-file.ttf text\n");
    exit (1);
  }

  fontfile = argv[1];
  text = argv[2];

  /* Initialize FreeType and create FreeType font face. */
  FT_Library ft_library;
  FT_Face ft_face;
  FT_Error ft_error;

  if ((ft_error = FT_Init_FreeType (&ft_library)))
    abort();
  if ((ft_error = FT_New_Face (ft_library, fontfile, 0, &ft_face)))
    abort();
  if ((ft_error = FT_Set_Char_Size (ft_face, FONT_SIZE*64, FONT_SIZE*64, 0, 0)))
    abort();

  /* Create hb-ft font. */
  hb_font_t *hb_font;
  hb_font = hb_ft_font_create (ft_face, NULL);

  /* Create hb-buffer and populate. */
  hb_buffer_t *hb_buffer;
  hb_buffer = hb_buffer_create ();
  hb_buffer_add_utf8(hb_buffer, text, -1, 0, -1);
  hb_buffer_guess_segment_properties (hb_buffer);

  /* Shape it! */
  hb_shape (hb_font, hb_buffer, NULL, 0);

  /* Print glyph names and positions out. */
  unsigned int len;
  hb_glyph_info_t *infos;
  hb_glyph_position_t *positions;
  len = hb_buffer_get_length (hb_buffer);
  infos = hb_buffer_get_glyph_infos (hb_buffer, NULL);
  positions = hb_buffer_get_glyph_positions (hb_buffer, NULL);

  for (unsigned int i = 0; i < len; i++)
  {
    hb_glyph_info_t *info = &infos[i];
    hb_glyph_position_t *pos = &positions[i];

    char glyphname[32];
    hb_font_get_glyph_name (hb_font, info->codepoint, glyphname, sizeof (glyphname));
    printf ("glyph '%s'	cluster %d	x-advance %4.2g offset %g,%g\n",
            glyphname,
	    info->cluster,
	    pos->x_advance / 64.,
	    pos->x_offset / 64.,
	    pos->y_offset / 64.);
  }

  /* Draw */
  int width = 2 * MARGIN;
  int height = 2 * MARGIN;
  for (unsigned int i = 0; i < len; i++)
  {
    width += positions[i].x_advance / 64.;
    height -= positions[i].y_advance / 64.;
  }
  if (HB_DIRECTION_IS_HORIZONTAL (hb_buffer_get_direction (hb_buffer)))
    height += FONT_SIZE;
  else
    width += FONT_SIZE;

  int image_len = width * height;
  unsigned char *image = (unsigned char*)calloc (image_len, sizeof (char));

  FT_GlyphSlot slot = ft_face->glyph;

  int x = MARGIN, y = MARGIN;
  if (HB_DIRECTION_IS_HORIZONTAL (hb_buffer_get_direction (hb_buffer)))
    y += (FONT_SIZE - ft_face->size->metrics.height / 64.) * .5 +
      ft_face->size->metrics.ascender / 64.;
  else
    x += FONT_SIZE * .5;

  for (int i = 0; i < len; ++i)
  {
    FT_Load_Glyph (ft_face, infos[i].codepoint, FT_LOAD_DEFAULT);
    FT_Render_Glyph (slot, FT_RENDER_MODE_NORMAL);

    const int src_x = x + (positions[i].x_offset / 64.) + slot->bitmap_left;
    const int src_y = y - (positions[i].y_offset / 64.) - slot->bitmap_top;
    const int src_height = slot->bitmap.rows;
    const int src_width = slot->bitmap.width;
    const int width_diff = width - src_width;
    int dst_index = src_y * width + src_x;
    int src_index = 0;
    for (int row = 0; row < src_height; dst_index += width_diff, ++row)
      for (int index = 0; index < src_width; ++index, ++dst_index, ++src_index)
        if (dst_index < image_len && dst_index > 0)
          image[dst_index] |= slot->bitmap.buffer[src_index];

    x += positions[i].x_advance / 64.;
    y -= positions[i].y_advance / 64.;
  }

#ifndef PNG_OUTPUT
  // Save as pbm file, Inkscape Libre/OpenOffice Draw can open it
  FILE *f = fopen ("out.pbm", "wb");
  fprintf (f, "P5 %d %d %d\n", width, height, slot->bitmap.num_grays - 1);
  fwrite ((const char *)image, sizeof (char), width * height, f);
  fclose (f);
#else
  stbi_write_png ("out.png", width, height, 1, image, 0);
#endif
  
  free (image);

  hb_buffer_destroy (hb_buffer);
  hb_font_destroy (hb_font);

  FT_Done_Face (ft_face);
  FT_Done_FreeType (ft_library);

  return 0;
}
