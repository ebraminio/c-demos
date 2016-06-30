// Ebrahim Byagowi <ebrahim@gnu.org>
#include <stdio.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include <hb.h>
#include <hb-ft.h>

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

// from Mozilla Firefox, https://bug908503.bmoattachments.org/attachment.cgi?id=8424620 
#pragma pack(1)
struct COLRBaseGlyphRecord {
  uint16_t    glyphId;
  uint16_t    firstLayerIndex;
  uint16_t    numLayers;
};

struct COLRLayerRecord {
  uint16_t    glyphId;
  uint16_t    paletteEntryIndex;
};

struct CPALColorRecord {
  uint8_t              blue;
  uint8_t              green;
  uint8_t              red;
  uint8_t              alpha;
};

struct COLRHeader {
  uint16_t    version;
  uint16_t    numBaseGlyphRecord;
  uint32_t    offsetBaseGlyphRecord;
  uint32_t    offsetLayerRecord;
  uint16_t    numLayerRecords;
};

struct CPALHeaderVersion0 {
  uint16_t    version;
  uint16_t    numPaletteEntries;
  uint16_t    numPalettes;
  uint16_t    numColorRecords;
  uint32_t    offsetFirstColorRecord;
};
#pragma pack()

static int
CompareBaseGlyph(const void* key, const void* data)
{
  uint32_t glyphId = (uint32_t)(uintptr_t)key;
  auto baseGlyph = (COLRBaseGlyphRecord*)data;
  uint16_t baseGlyphId = hb_uint16_swap((uint16_t)(baseGlyph->glyphId));

  if (baseGlyphId == glyphId) {
    return 0;
  }

  return baseGlyphId > glyphId ? -1 : 1;
}

bool GetColorLayersInfo(hb_face_t *face,
  uint32_t glyphId,
  uint16_t* layerGlyphs[],
  uint32_t* layerColors[],
  int* layers)
{
  COLRHeader* colr = (COLRHeader*)hb_blob_get_data(
    hb_face_reference_table(face, hb_tag_from_string("COLR", -1)), 0);
  CPALHeaderVersion0* cpal = (CPALHeaderVersion0*)hb_blob_get_data(
    hb_face_reference_table(face, hb_tag_from_string("CPAL", -1)), 0);

  const uint8_t* baseGlyphRecords =
    (const uint8_t*)colr + uint32_t(hb_uint32_swap(colr->offsetBaseGlyphRecord));
  // BaseGlyphRecord is sorted by glyphId
  COLRBaseGlyphRecord* baseGlyph = (COLRBaseGlyphRecord*)bsearch(
    (void*)(uintptr_t)glyphId,
    baseGlyphRecords,
    uint16_t(hb_uint16_swap(colr->numBaseGlyphRecord)),
    sizeof(COLRBaseGlyphRecord),
    CompareBaseGlyph);
  if (!baseGlyph) { return false; }

  const COLRLayerRecord* layer = (COLRLayerRecord*)(((uint8_t*)colr) +
    uint32_t(hb_uint32_swap(colr->offsetLayerRecord)) +
    sizeof(COLRLayerRecord) * uint16_t(hb_uint16_swap(baseGlyph->firstLayerIndex)));
  const uint16_t numLayers = hb_uint16_swap(baseGlyph->numLayers);
  const uint32_t offsetFirstColorRecord = hb_uint32_swap(cpal->offsetFirstColorRecord);

  *layerGlyphs = (uint16_t*)malloc(numLayers * sizeof(uint16_t));
  *layerColors = (uint32_t*)malloc(numLayers * sizeof(uint32_t));
  *layers = numLayers;

  for (uint16_t layerIndex = 0; layerIndex < numLayers; layerIndex++) {
    (*layerGlyphs)[layerIndex] = hb_uint16_swap(layer->glyphId);
    CPALColorRecord* color = (CPALColorRecord*)(((uint8_t*)cpal) +
      offsetFirstColorRecord +
      sizeof(CPALColorRecord) * hb_uint16_swap(layer->paletteEntryIndex));
    (*layerColors)[layerIndex] = (uint32_t)color->red + (color->green << 8)
      + (color->blue << 16) + (color->alpha << 24);
    layer++;
  }

  return true;
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
    "colrcpalfont.ttf",
    0,
    &face);

  FT_Set_Char_Size(face, 0, 29100, 72, 72);

  // Configure harfbuzz buffer and shape
  hb_font_t *hb_font = hb_ft_font_create(face, NULL);
  hb_face_t *hb_face = hb_font_get_face(hb_font);

  for (int i = 0; i < face->num_glyphs; ++i) {
    
    uint16_t* layerGlyphs;
    uint32_t* layerColors;
    int layers;
    if (GetColorLayersInfo(hb_face, i, &layerGlyphs, &layerColors, &layers)) {

      FT_Load_Glyph(face, i, FT_LOAD_DEFAULT);
      FT_GlyphSlot slot = face->glyph;
      FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL);
      int left = slot->bitmap_left;
      int top = slot->bitmap_top;

      int width = slot->bitmap.width;
      int height = slot->bitmap.rows;

      unsigned char* image = (unsigned char*)calloc(width * height * 16, sizeof(char));

      for (int n = 0; n < layers; ++n) {
        FT_Load_Glyph(face, layerGlyphs[n], FT_LOAD_DEFAULT);
        FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL);
        drawBitmap(&slot->bitmap,
          slot->bitmap_left - left,
          top - slot->bitmap_top,
          image, width, height, layerColors[n]);
      }

      char outName[255];
      sprintf(outName, "out/%d.png", i);
      stbi_write_png(outName, width, height, 1 * 4, image, 0);

      free(image);
      free(layerGlyphs);
      free(layerColors);
    }
  }

  // Destroying things
  FT_Done_Face(face);
  FT_Done_FreeType(library);

  return 0;
}
