// gcc raqm-shape.c `pkg-config --cflags --libs raqm` -Wall && ./a.out
#include <stdio.h>

#include <raqm.h>

int main() {
  FT_Library library;
  FT_Init_FreeType(&library);
  FT_Face face;
  FT_New_Face(library, "input/Roboto.abc.ttf", 0, &face);
  FT_Set_Char_Size(face, 36 * 64, 36 * 64, 0, 0);
  raqm_t *rq = raqm_create();
  const char *text = "abc";
  raqm_set_text_utf8(rq, text, strlen(text));
  raqm_set_freetype_face(rq, face);
  raqm_set_par_direction(rq, RAQM_DIRECTION_LTR);
  const char *lang = "en";
  raqm_set_language(rq, lang, 0, strlen(lang));
  raqm_layout(rq);
  size_t count;
  raqm_glyph_t *glyphs = raqm_get_glyphs(rq, &count);
  for (unsigned i = 0; i < count; i++)
    printf("gid#%d off: (%d, %d) adv: (%d, %d) idx: %d\n", glyphs[i].index,
           glyphs[i].x_offset, glyphs[i].y_offset, glyphs[i].x_advance,
           glyphs[i].y_advance, glyphs[i].cluster);
  printf("\n");
  raqm_destroy(rq);
  FT_Done_Face(face);
  FT_Done_FreeType(library);
  return 0;
}