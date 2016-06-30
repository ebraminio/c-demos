// Ebrahim Byagowi <ebrahim@gnu.org>
#include <stdio.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include <hb.h>
#include <hb-ft.h>
#include <hb-directwrite.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// from harfbuzz
static inline uint16_t hb_uint16_swap(const uint16_t v)
{
  return (v >> 8) | (v << 8);
}
static inline uint32_t hb_uint32_swap(const uint32_t v)
{
  return (hb_uint16_swap(v) << 16) | hb_uint16_swap(v >> 16);
}

// Simple pixel copy code brought from http://www.freetype.org/freetype2/docs/tutorial/example1.c
inline void drawBitmap(FT_Bitmap* bitmap, FT_Int x, FT_Int y, unsigned char* image, int width, int height, uint32_t color) {
  FT_Int i, j, p, q;
  FT_Int x_max = x + bitmap->width;
  FT_Int y_max = y + bitmap->rows;

  int r = color & 0xFF;
  int g = (color >> 8) & 0xFF;
  int b = (color >> 16) & 0xFF;
  int alpha = (color >> 24) & 0xFF;

  for (i = x, p = 0; i < x_max; i++, p++) {
    for (j = y, q = 0; j < y_max; j++, q++) {
      if (i < 0 || j < 0 ||
        i >= width || j >= height)
        continue;

      // https://github.com/ShaoYuZhang/freetype/commit/27d9e6f6012b55d4001c33e3d46f6814c94ebc0d#diff-3f6d401db89b11a93b8eec6805f53a21R423
      int aa = bitmap->buffer[q * bitmap->width + p];
      int fa = alpha * aa / 255;
      int fr = r * fa / 255;
      int fg = g * fa / 255;
      int fb = b * fa / 255;
      int ba2 = 255 - fa;
      int br = image[4 * j * width + i * 4 + 0];
      int bg = image[4 * j * width + i * 4 + 1];
      int bb = image[4 * j * width + i * 4 + 2];
      int ba = image[4 * j * width + i * 4 + 3];
      image[4 * j * width + i * 4 + 0] = br * ba2 / 255 + fr;
      image[4 * j * width + i * 4 + 1] = bg * ba2 / 255 + fg;
      image[4 * j * width + i * 4 + 2] = bb * ba2 / 255 + fb;
      image[4 * j * width + i * 4 + 3] = ba * ba2 / 255 + fa;
    }
  }
}

int main() {
  // Initialize freetype
  FT_Library library;
  FT_Init_FreeType(&library);

  // Initialize the font face
  FT_Face face;
  FT_New_Face(library,
    "C:\\Windows\\Fonts\\tahoma.ttf",
    0,
    &face);

  FT_Set_Char_Size(face, 0, 256 * 64, 72, 72);
  // Configure harfbuzz buffer and shape
  hb_font_t *hb_font;
  hb_face_t *hb_face;
  hb_buffer_t *buf = nullptr;
  const char *text = "متن"; // Emoji sequence

  auto a = new char*{ "directwrite" };
  for (int i = 0; i < 10; ++i) {
    if (buf) {
      hb_buffer_destroy(buf);
      //hb_face_destroy(hb_face);
      hb_font_destroy(hb_font);
    }
    hb_font = hb_ft_font_create(face, NULL);
    hb_face = hb_font_get_face(hb_font);
    buf = hb_buffer_create();
    hb_buffer_set_direction(buf, HB_DIRECTION_RTL);
    hb_buffer_set_language(buf, hb_language_from_string("fa", 2));
    hb_buffer_set_script(buf, HB_SCRIPT_ARABIC);
    hb_buffer_add_utf8(buf, text, strlen(text), 0, -1);
    hb_directwrite_buffer_set_line_width(buf, 1000);
    hb_shape_full(hb_font, buf, 0, 0, a);
  }
  delete a;
  

  unsigned int glyphCount;

  hb_glyph_info_t *glyphInfos = hb_buffer_get_glyph_infos(buf, &glyphCount);
  hb_glyph_position_t *glyphPositions = hb_buffer_get_glyph_positions(buf, &glyphCount);

  // this is not a good assumption specially when there is GPOS on the font
  int height = face->size->metrics.height / 64;
  int width = 0;
  // doing width measuring just based shaper glyphs advances is not good assumption either
  for (int i = 0; i < glyphCount; ++i) { width += glyphPositions[i].x_advance; }
  width /= 64;
  unsigned char* image = (unsigned char*)calloc(width * height * 16, sizeof(char));
  FT_GlyphSlot slot = face->glyph;
  int x = 0, y = face->size->metrics.ascender / 64;
  for (int i = 0; i < glyphCount; ++i) {
    uint16_t* layerGlyphs = NULL;
    uint32_t* layerColors = NULL;

    FT_Load_Glyph(face, glyphInfos[i].codepoint, FT_LOAD_DEFAULT);
    FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL);
    drawBitmap(&slot->bitmap,
      x + (glyphPositions[i].x_offset / 64) + slot->bitmap_left,
      y - (glyphPositions[i].y_offset / 64) - slot->bitmap_top,
      image, width, height, 0xFFFFFFFF);
    int a = 2;

    x += glyphPositions[i].x_advance / 64;
    y -= glyphPositions[i].y_advance / 64;
  }

  stbi_write_png("out.png", width, height, 1 * 4, image, 0);

  // Destroying things
  free(image);
  
  hb_buffer_clear_contents(buf);
  hb_buffer_destroy(buf);
  hb_font_destroy(hb_font);
  FT_Done_Face(face);
  FT_Done_FreeType(library);

  return 0;
}
