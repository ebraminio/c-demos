// gcc libzstd-decode.c `pkg-config --libs --cflags libzstd` -Wall && ./a.out
#include <stdio.h>
#include <stdlib.h>

#include <zstd.h>

int main() {
  uint8_t *src;
  unsigned src_size;
  {
    FILE *f = fopen("input/test.txt.zst", "rb");
    fseek(f, 0, SEEK_END);
    src_size = ftell(f);
    rewind(f);
    src = (uint8_t *)malloc(src_size);
    src_size = fread(src, 1, src_size, f);
    fclose(f);
  }
  unsigned dst_size = ZSTD_getFrameContentSize(src, src_size);
  uint8_t *dst = malloc(dst_size);
  ZSTD_decompress(dst, dst_size, src, src_size);
  free(src);
  printf("%.*s", dst_size, dst);
  free(dst);
  return 0;
}
