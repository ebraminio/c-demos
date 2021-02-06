// gcc libwebp-decode.c `pkg-config --libs --cflags libwebp` -Wall && ./a.out
#include <stdio.h>
#include <stdlib.h>

#include <webp/decode.h>

int main() {
  uint8_t *buf;
  unsigned len;
  {
    FILE *f = fopen("input/test.webp", "rb");
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    rewind(f);
    buf = (uint8_t *)malloc(len);
    len = fread(buf, 1, len, f);
    fclose(f);
  }
  int width, height;
  uint8_t *image = WebPDecodeRGB(buf, len, &width, &height);
  free(buf);
  {
    FILE *f = fopen("out.pbm", "wb");
    fprintf(f, "P6 %d %d 255\n", width, height);
    fwrite(image, sizeof(char), width * height * 3, f);
    fclose(f);
  }
  WebPFree(image);
  return 0;
}
