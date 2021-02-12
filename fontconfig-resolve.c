// cc fontconfig-resolve.c `pkg-config --libs --cflags fontconfig` -Wall && ./a.out
// https://gist.github.com/CallumDev/7c66b3f9cf7a876ef75f#gistcomment-3355764
#include <stdio.h>
#include <stdlib.h>

#include <fontconfig/fontconfig.h>

int main() {
  FcInit();
  FcConfig *config = FcInitLoadConfigAndFonts();

  // not necessarily has to be a specific name
  FcPattern *pat = FcNameParse((const FcChar8 *)"Arial");

  // NECESSARY; it increases the scope of possible fonts
  FcConfigSubstitute(config, pat, FcMatchPattern);
  // NECESSARY; it increases the scope of possible fonts
  FcDefaultSubstitute(pat);

  char *fontFile;
  FcResult result;

  FcPattern *font = FcFontMatch(config, pat, &result);

  if (font) {
    FcChar8 *file = NULL;

    if (FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch) {
      fontFile = (char *)file;
      printf("%s\n", fontFile);
    }
  }
  FcPatternDestroy(font);
  FcPatternDestroy(pat);
  FcConfigDestroy(config);
  FcFini();
  return 0;
}