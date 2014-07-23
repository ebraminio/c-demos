// Simplified GLFW example
// Ebrahim Byagowi <ebrahim@gnu.org>
#include <stdio.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#define HAVE_UNISCRIBE
#include <hb.h>
#include <hb-ft.h>
#include <hb-uniscribe.h>

// Simple pixel copy code brought from http://www.freetype.org/freetype2/docs/tutorial/example1.c
void draw_bitmap(FT_Bitmap* bitmap, FT_Int x, FT_Int y, unsigned char* image, int width, int height)
{
	FT_Int i, j, p, q;
	FT_Int x_max = x + bitmap->width;
	FT_Int y_max = y + bitmap->rows;

	for (i = x, p = 0; i < x_max; i++, p++)
	{
		for (j = y, q = 0; j < y_max; j++, q++)
		{
			if (i < 0 || j < 0 ||
				i >= width || j >= height)
				continue;

			image[j * width + i] |= bitmap->buffer[q * bitmap->width + p];
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
// Just for sake of simplicity
#if defined(__linux__)
		"/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
#elif defined(_WIN32) || defined(_WIN64)
		"C:\\Windows\\Fonts\\tahoma.ttf",
#elif __APPLE__
		"/Library/Fonts/Tahoma.ttf",
#endif
		0,
		&face);

	FT_Set_Char_Size(face, 0, 50 * 64, 72, 72);

	// Configure harfbuzz buffer and shape
	hb_font_t *hbFont = hb_ft_font_create(face, NULL);
	hb_buffer_t *buf = hb_buffer_create();
	hb_buffer_set_direction(buf, HB_DIRECTION_RTL);
	hb_buffer_set_script(buf, HB_SCRIPT_ARABIC); // see hb-unicode.h
	// hb_buffer_set_language(buf, hb_language_from_string("ar", 2)); // optional but needed for advanced font features
	const char *text = "مَتن"; // Sample Arabic text
	hb_buffer_add_utf8(buf, text, strlen(text), 0, strlen(text));
	hb_shape(hbFont, buf, NULL, 0); // Shaping magic happens here

	// Drawing
	unsigned int glyphCount;
	hb_glyph_info_t *glyphInfos = hb_buffer_get_glyph_infos(buf, &glyphCount);
	hb_glyph_position_t *glyphPositions = hb_buffer_get_glyph_positions(buf, &glyphCount);

	// this is not a good assumption specially when there is GPOS on the font
	int height = (face->max_advance_height - face->descender) / 64 + 5;
	int width = 120; // for now
	unsigned char* image = (unsigned char*)malloc(sizeof(char) * width * height);
	memset(image, 0, width * height);
	FT_GlyphSlot slot = face->glyph;
	int x = 0, y = face->max_advance_height / 64 + 2;
	for (int i = 0; i < glyphCount; ++i) {
		FT_Load_Glyph(face, glyphInfos[i].codepoint, FT_LOAD_RENDER);

		draw_bitmap(&slot->bitmap,
			x + (glyphPositions[i].x_offset / 64) + slot->bitmap_left,
			y - (glyphPositions[i].y_offset / 64) - slot->bitmap_top,
			image, width, height);

		x += glyphPositions[i].x_advance / 64;
		y -= glyphPositions[i].y_advance / 64;
	}

	// Saving as pbm file, Inkscape can open it
	FILE *f = fopen("out.pbm", "wb");
	fprintf(f, "P5 %d %d %d\n", width, height, slot->bitmap.num_grays - 1);
	fwrite((const char *)image, sizeof(char), width * height, f);
	fclose(f);

	// Destroying things
	free(image);
	hb_buffer_clear_contents(buf);
	hb_buffer_destroy(buf);
	FT_Done_Face(face);
	FT_Done_FreeType(library);

	return 0;
}