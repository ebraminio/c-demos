// clang libjpeg-decode.c `pkg-config --libs --cflags libturbojpeg` -Wall && ./a.out
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jpeglib.h>

uint8_t *decode(uint8_t *buf, unsigned buf_size, unsigned *width,
                unsigned *height) {
  struct jpeg_decompress_struct cinfo;
  jpeg_create_decompress(&cinfo);
  jpeg_mem_src(&cinfo, buf, buf_size);
  struct jpeg_error_mgr jerr;
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_read_header(&cinfo, TRUE);
  jpeg_start_decompress(&cinfo);
  *width = cinfo.output_width;
  *height = cinfo.output_height;
  int row_stride = cinfo.output_width * cinfo.output_components;
  (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);
  uint8_t *result = (uint8_t *)malloc(cinfo.output_height * row_stride);
  while (cinfo.output_scanline < cinfo.output_height) {
    uint8_t *buffer_array = result + (cinfo.output_scanline) * row_stride;
    jpeg_read_scanlines(&cinfo, &buffer_array, 1);
  }
  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
  return result;
}

int main() {
  uint8_t *buf;
  unsigned len;
  {
    FILE *f = fopen("input/testorig.jpg", "rb");
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    rewind(f);
    buf = (uint8_t *)malloc(len);
    len = fread(buf, 1, len, f);
    fclose(f);
  }
  unsigned width, height;
  uint8_t *image = decode(buf, len, &width, &height);
  free(buf);
  {
    FILE *f = fopen("out.pbm", "wb");
    fprintf(f, "P6 %d %d 255\n", width, height);
    fwrite(image, sizeof(char), width * height * 3, f);
    fclose(f);
  }
  free(image);
  return 0;
}
