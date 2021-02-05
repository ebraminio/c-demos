// gcc freetype-render.c `pkg-config --libs --cflags freetype2` -Wall && a.out
#include <stdio.h>
#include <stdlib.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

int main() {
  FT_Library library;
  FT_Init_FreeType(&library);
  FT_Face face;
  FT_New_Face(library, "input/Roboto.abc.ttf", 0, &face);
  FT_Set_Char_Size(face, 36 * 64, 36 * 64, 0, 0);

  FT_Load_Glyph(face, 2 /* gid=3 is 'b' in this particular font */,
                FT_LOAD_DEFAULT);

  FT_GlyphSlot slot = face->glyph;
  FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL);

  unsigned width = slot->bitmap.width;
  unsigned height = slot->bitmap.rows;

  FILE *f = fopen("out.pbm", "wb");
  fprintf(f, "P5 %d %d %d\n", width, height, slot->bitmap.num_grays - 1);
  fwrite(slot->bitmap.buffer, sizeof(char), width * height, f);
  fclose(f);

  FT_Done_Face(face);
  FT_Done_FreeType(library);
  return 0;
}
