// gcc libpng-decode.c `pkg-config --libs --cflags libpng` -Wall && ./a.out
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <png.h>

typedef struct {
  uint8_t *buf;
  unsigned len;
  unsigned location;
} png_memory_reader;

void read_function(png_structp ps, png_bytep data, png_size_t length) {
  png_memory_reader *reader = png_get_io_ptr(ps);
  memcpy(data, reader->buf + reader->location,
         reader->location + length <= reader->len ? length : 0);
  reader->location += length;
}

uint8_t *decode(uint8_t *buf, unsigned buf_size, uint32_t *width,
                uint32_t *height, int *color_type) {
  if (png_sig_cmp(buf, 0, 8))
    abort();
  png_structp png_ptr =
      png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (setjmp(png_jmpbuf(png_ptr)))
    abort();

  png_memory_reader memory_reader = {
      .location = 0, .buf = buf, .len = buf_size};
  png_set_read_fn(png_ptr, &memory_reader, read_function);
  png_read_info(png_ptr, info_ptr);

  png_get_IHDR(png_ptr, info_ptr, width, height, NULL, color_type, NULL, NULL,
               NULL);
  png_set_expand(png_ptr);
  png_set_strip_16(png_ptr);
  png_set_gray_to_rgb(png_ptr);
  png_set_interlace_handling(png_ptr);

  png_read_update_info(png_ptr, info_ptr);

  if (setjmp(png_jmpbuf(png_ptr)))
    abort();

  png_bytep *row_pointers = malloc(sizeof(png_bytep) * *height);
  unsigned rowbytes = png_get_rowbytes(png_ptr, info_ptr);
  uint8_t *result = malloc(rowbytes * *height);
  for (unsigned y = 0; y < *height; ++y)
    row_pointers[y] = result + rowbytes * y;

  png_read_image(png_ptr, row_pointers);
  png_read_end(png_ptr, NULL);

  return result;
}

int main(int argc, char **argv) {
  uint8_t *buf;
  unsigned len;
  {
    FILE *f = fopen(argc == 2 ? argv[1] : "input/test.png", "rb");
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    rewind(f);
    buf = (uint8_t *)malloc(len);
    len = fread(buf, 1, len, f);
    fclose(f);
  }
  unsigned width, height;
  int color_type;
  uint8_t *image = decode(buf, len, &width, &height, &color_type);
  free(buf);

  if (color_type != PNG_COLOR_TYPE_RGB_ALPHA) {
    printf("Only RGBA types are supported for the rest of the example for now");
    return 1;
  }
  {
    // Just that .pbm doesn't supoprt 4 channels let's turn it into 3 channels
    // https://stackoverflow.com/a/18495447
    for (int i = 0; i < width * height; i++) {
      int r = image[i * 4 + 0], g = image[i * 4 + 1], b = image[i * 4 + 2],
          a = image[i * 4 + 3];

      image[i * 3 + 0] = r * a / 255;
      image[i * 3 + 1] = g * a / 255;
      image[i * 3 + 2] = b * a / 255;
    }
  }

  {
    FILE *f = fopen("out.pbm", "wb");
    fprintf(f, "P6 %d %d 255\n", width, height);
    fwrite(image, sizeof(char), width * height * 3, f);
    fclose(f);
  }

  free(image);
  return 0;
}
