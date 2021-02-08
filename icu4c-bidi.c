
// gcc icu4c-bidi.c `pkg-config --cflags --libs icu-i18n` -Wall && ./a.out
#include <stdio.h>
#include <stdlib.h>

#include <unicode/ubidi.h>

int main() {
  UChar text[] = u"abcششa234 سیبسی ۲۳۴ سیشبسبbc";
  unsigned length = sizeof(text) / sizeof(UChar);
  UErrorCode errorCode = U_ZERO_ERROR;
  UBiDi *para = ubidi_openSized(length, 0, &errorCode);
  if (!para) return 1;
  ubidi_setPara(para, text, length, UBIDI_DEFAULT_LTR, NULL,
                &errorCode);
  const UBiDiLevel *levels = ubidi_getLevels(para, &errorCode);
  if (U_FAILURE(errorCode)) return 1;
  for (unsigned i = 0; i < length; ++i)
    printf(i ? ",%d" : "%d", levels[i]);
  // Result: 0,0,0,1,1,0,0,0,0,0,1,1,1,1,1,1,2,2,2,1,1,1,1,1,1,1,0,0,0
  // Even numbers for LTR characters and odd numbers for RTL
  // Higher numbers mean they are in more inner levels
  printf("\n");
  ubidi_close(para);
  return 0;
}
