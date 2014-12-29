// HarfBuzz sample of SVG rendering. It is not best practice nor right way to do
// text rendering using harfbuzz/ft. There are more superior sample on the net,
// you should look at for a real example, this one is just a fun one
// Ebrahim Byagowi <ebrahim@gnu.org>
//
// sudo apt-get install libfreetype6-dev libharfbuzz-dev
// Compile and run on Chrome:
// g++ hbftsvg.cc `pkg-config --cflags --libs freetype2 harfbuzz` && chromium-browser 1.svg
//

#include <cstdio>
#include <fstream>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include <hb.h>
#include <hb-ft.h>

std::string do_outline(FT_Vector_ *ftpoints, FT_Outline ftoutline, char* tagS, short* contourS);

int main() {
  // Initialize freetype
  FT_Library library;
  FT_Init_FreeType(&library);

  // Initialize the font face
  FT_Face face;
  FT_New_Face(library,
    // Just for sake of simplicity
#if 0
    "IranNastaliq.ttf",
#else
  #if defined(__linux__)
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
  #elif defined(_WIN32) || defined(_WIN64)
    "C:\\Windows\\Fonts\\tahoma.ttf",
  #elif __APPLE__
    "/Library/Fonts/Tahoma.ttf",
  #endif
#endif
    0,
    &face);

  hb_face_t *hb_face = hb_ft_face_create(face, NULL);
  hb_font_t *font = hb_font_create(hb_face);
  unsigned int upem = hb_face_get_upem(hb_face);
  hb_font_set_scale(font, upem, upem);
  hb_ft_font_set_funcs(font);

  hb_buffer_t *buf = hb_buffer_create();
  hb_buffer_set_direction(buf, HB_DIRECTION_RTL);
  hb_buffer_set_script(buf, HB_SCRIPT_ARABIC); // see hb-unicode.h
  // hb_buffer_set_language(buf, hb_language_from_string("ar", 2)); // for language specific font features
  const char *text = "طرح‌نَما"; // Arabic script sample
  hb_buffer_add_utf8(buf, text, strlen(text), 0, strlen(text));
  hb_shape(font, buf, NULL, 0); // Shaping magic happens here

  // Drawing
  unsigned int glyphCount;
  hb_glyph_info_t *glyphInfos = hb_buffer_get_glyph_infos(buf, &glyphCount);
  hb_glyph_position_t *glyphPositions = hb_buffer_get_glyph_positions(buf, &glyphCount);

  // this is not a good assumption specially when there is GPOS on the font
  int height = face->height;
  int width = 0;
  for (int i = 0; i < glyphCount; ++i) { width += glyphPositions[i].x_advance; }
  int x = 0, y = face->ascender;

  std::ofstream file("1.svg");
  file << "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 " << width << " " << height << "\" height=\"700\">";

  for (int i = 0; i < glyphCount; ++i) {
    FT_Load_Glyph(face, glyphInfos[i].codepoint, FT_LOAD_NO_SCALE |
      FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT | FT_LOAD_NO_BITMAP |
      FT_LOAD_LINEAR_DESIGN | FT_KERNING_UNSCALED | FT_LOAD_IGNORE_TRANSFORM);

    FT_GlyphSlot slot = face->glyph;
    FT_Outline ftoutline = slot->outline;
    FT_Glyph_Metrics gm = slot->metrics;

    int s = x + glyphPositions[i].x_offset + slot->bitmap_left;
    int t = y - glyphPositions[i].y_offset - slot->bitmap_top;

    FT_Vector_ *ftpoints = ftoutline.points;

    // TODO: Instead do_outline, use FT_Outline_Decompose based approach
#if 0
    for (int j = 0; j < ftoutline.n_points; j++) {
      ftpoints[j].y *= -1;
      ftpoints[j].x += s;
      ftpoints[j].y += t;
    }
    file << do_outline(ftpoints, ftoutline, ftoutline.tags, ftoutline.contours).c_str();
#else
    file << "<g transform=\"translate(" << s << ", " << t << ") scale(1, -1)\">";
    file << do_outline(ftpoints, ftoutline, ftoutline.tags, ftoutline.contours).c_str();
    file << "</g>";
#endif

    x += glyphPositions[i].x_advance;
    y -= glyphPositions[i].y_advance;

    file << std::endl;
  }

  file << std::endl << "</svg>";
  file.close();

  // Destroying things
  hb_buffer_clear_contents(buf);
  hb_buffer_destroy(buf);
  FT_Done_Face(face);
  FT_Done_FreeType(library);

  return 0;
}



// Below code is brought from this project:
// github.com/donbright/font_to_svg (Under BSD license)
#include <vector>
#include <iostream>
#include <sstream>
/* Draw the outline of the font as svg.
There are three main components.
1. the points
2. the 'tags' for the points
3. the contour indexes (that define which points belong to which contour)
*/
std::string do_outline(FT_Vector_ *ftpoints, FT_Outline ftoutline, char* tagS, short* contourS)
{
  std::vector<FT_Vector> points(ftpoints, ftpoints + ftoutline.n_points);
  std::vector<char> tags(tagS, tagS + ftoutline.n_points);
  std::vector<short> contours(contourS, contourS + ftoutline.n_contours);

  std::stringstream debug, svg;
  if (points.size() == 0) return "<!-- font had 0 points -->";
  if (contours.size() == 0) return "<!-- font had 0 contours -->";
  svg.str("");
  svg << "\n  <path d='";

  /* tag bit 1 indicates whether its a control point on a bez curve
  or not. two consecutive control points imply another point halfway
  between them */

  // Step 1. move to starting point (M x-coord y-coord )
  // Step 2. decide whether to draw a line or a bezier curve or to move
  // Step 3. for bezier: Q control-point-x control-point-y,
  //             destination-x, destination-y
  //         for line:   L x-coord, y-coord
  //         for move:   M x-coord, y-coord

  int contour_starti = 0;
  int contour_endi = 0;
  for (int i = 0; i < contours.size(); i++) {
    contour_endi = contours.at(i);
    debug << "new contour starting. startpt index, endpt index:";
    debug << contour_starti << "," << contour_endi << "\n";
    int offset = contour_starti;
    int npts = contour_endi - contour_starti + 1;
    debug << "number of points in this contour: " << npts << "\n";
    debug << "moving to first pt " << points[offset].x << "," << points[offset].y << "\n";
    svg << "\n M " << points[contour_starti].x << "," << points[contour_starti].y << "\n";
    debug << "listing pts: [this pt index][isctrl] <next pt index><isctrl> [x,y] <nx,ny>\n";
    for (int j = 0; j < npts; j++) {
      int thisi = j%npts + offset;
      int nexti = (j + 1) % npts + offset;
      int nextnexti = (j + 2) % npts + offset;
      int x = points[thisi].x;
      int y = points[thisi].y;
      int nx = points[nexti].x;
      int ny = points[nexti].y;
      int nnx = points[nextnexti].x;
      int nny = points[nextnexti].y;
      bool this_tagbit1 = (tags[thisi] & 1);
      bool next_tagbit1 = (tags[nexti] & 1);
      bool nextnext_tagbit1 = (tags[nextnexti] & 1);
      bool this_isctl = !this_tagbit1;
      bool next_isctl = !next_tagbit1;
      bool nextnext_isctl = !nextnext_tagbit1;
      debug << " [" << thisi << "]";
      debug << "[" << !this_tagbit1 << "]";
      debug << " <" << nexti << ">";
      debug << "<" << !next_tagbit1 << ">";
      debug << " <<" << nextnexti << ">>";
      debug << "<<" << !nextnext_tagbit1 << ">>";
      debug << " [" << x << "," << y << "]";
      debug << " <" << nx << "," << ny << ">";
      debug << " <<" << nnx << "," << nny << ">>";
      debug << "\n";

      if (this_isctl && next_isctl) {
        debug << " two adjacent ctl pts. adding point halfway between " << thisi << " and " << nexti << ":";
        debug << " reseting x and y to ";
        x = (x + nx) / 2;
        y = (y + ny) / 2;
        this_isctl = false;
        debug << " [" << x << "," << y << "]\n";
        if (j == 0) {
          debug << "first pt in contour was ctrl pt. moving to non-ctrl pt\n";
          svg << " M " << x << "," << y << "\n";
        }
      }

      if (!this_isctl && next_isctl && !nextnext_isctl) {
        svg << " Q " << nx << "," << ny << " " << nnx << "," << nny << "\n";
        debug << " bezier to " << nnx << "," << nny << " ctlx, ctly: " << nx << "," << ny << "\n";
      }
      else if (!this_isctl && next_isctl && nextnext_isctl) {
        debug << " two ctl pts coming. adding point halfway between " << nexti << " and " << nextnexti << ":";
        debug << " reseting nnx and nny to halfway pt";
        nnx = (nx + nnx) / 2;
        nny = (ny + nny) / 2;
        svg << " Q " << nx << "," << ny << " " << nnx << "," << nny << "\n";
        debug << " bezier to " << nnx << "," << nny << " ctlx, ctly: " << nx << "," << ny << "\n";
      }
      else if (!this_isctl && !next_isctl) {
        svg << " L " << nx << "," << ny << "\n";
        debug << " line to " << nx << "," << ny << "\n";
      }
      else if (this_isctl && !next_isctl) {
        debug << " this is ctrl pt. skipping to " << nx << "," << ny << "\n";
      }
    }
    contour_starti = contour_endi + 1;
    svg << " Z\n";
  }
  svg << "\n  '/>";
  return svg.str();
}
