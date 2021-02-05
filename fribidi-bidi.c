// gcc fribidi-bidi.c `pkg-config --libs --cflags fribidi` -Wall && ./a.out
#include <stdio.h>
#include <stdlib.h>

#include <fribidi.h>

int main() {
  uint32_t str[] = U"abcششa234 سیبسی ۲۳۴ سیشبسبbc";
  unsigned str_len = sizeof(str) / sizeof(uint32_t);

  FriBidiCharType *types = malloc(str_len * sizeof(FriBidiCharType));
  FriBidiBracketType *btypes = malloc(str_len * sizeof(FriBidiBracketType));
  FriBidiLevel *levels = malloc(str_len * sizeof(FriBidiLevel));
  FriBidiParType pbase_dir;

  fribidi_get_bidi_types(str, str_len, types);
  fribidi_get_bracket_types(str, str_len, types, btypes);
  unsigned max_level = fribidi_get_par_embedding_levels_ex(
      types, btypes, str_len, &pbase_dir, levels);

  for (unsigned i = 0; i < str_len; ++i)
    printf(i ? ",%d" : "%d", levels[i]);

  // Result: 0,0,0,1,1,0,0,0,0,0,1,1,1,1,1,1,2,2,2,1,1,1,1,1,1,1,0,0,0
  // Even numbers for LTR characters and odd numbers for RTL
  // Higher numbers mean they are in more inner levels

  printf("\nmax level: %d\n", max_level);

  free(types);
  free(btypes);
  free(levels);
  return 0;
}