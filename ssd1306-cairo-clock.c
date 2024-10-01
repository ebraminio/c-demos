// Draw a digital clock using Cairo on SSD1306 LCD in a Raspberry PI
// $ cc ssd1306-cairo-clock.c `pkg-config --cflags --libs cairo` && ./a.out
// Originally based on https://github.com/pive/rpi1306i2c/blob/master/rpi1306i2c.hpp

#include <assert.h>
#include <fcntl.h>
#include <cairo.h>
#include <linux/i2c-dev.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#define MEMORYMODE          0x20
#define COLUMNADDR          0x21
#define PAGEADDR            0x22
#define SETSTARTLINE        0x40
#define SETCONTRAST         0x81
#define CHARGEPUMP          0x8D
#define SEGREMAP            0xA0
#define DISPLAYALLON_RESUME 0xA4
#define DISPLAYALLON        0xA5
#define NORMALDISPLAY       0xA6
#define SETMULTIPLEX        0xA8
#define DISPLAYOFF          0xAE
#define DISPLAYON           0xAF
#define SETPAGE             0xB0
#define COMSCANINC          0xC0
#define COMSCANDEC          0xC8
#define SETDISPLAYOFFSET    0xD3
#define SETDISPLAYCLOCKDIV  0xD5
#define SETPRECHARGE        0xD9
#define SETCOMPINS          0xDA
#define SETVCOMDETECT       0xDB

const uint8_t ssd1306_128x32_init_seq[] = {
  0x80, DISPLAYOFF,
  0x80, SETDISPLAYCLOCKDIV, 0x80, 0x80,
  0x80, SETMULTIPLEX, 0x80, 0x1F,
  0x80, SETDISPLAYOFFSET, 0x80, 0x00,
  0x80, SETSTARTLINE | 0x00,
  0x80, CHARGEPUMP, 0x80, 0x14,
  0x80, SEGREMAP,
  0x80, COMSCANDEC,
  0x80, SETCOMPINS, 0x80, 0x12,
  0x80, SETCONTRAST, 0x80, 0x1,//0x7F, brightness
  0x80, SETPRECHARGE, 0x80, 0x22,
  0x80, SETVCOMDETECT, 0x80, 0x20,
  0x80, MEMORYMODE,
  0x80, DISPLAYALLON_RESUME,
  0x80, NORMALDISPLAY,
  0x80, COLUMNADDR, 0x80, 0, 0x80, 128 - 1,
  0x80, PAGEADDR, 0x80, 0, 0x80, (32 >> 3) - 1,
  0x80, DISPLAYON,
};

static void writeBuffer(int deviceNode, const uint8_t *buffer, unsigned size) {
  if (write(deviceNode, buffer, size) != size) {
    printf("Could not write on the device");
    abort();
  }
}

static void debug(uint8_t *buffer, unsigned stride) {
  for (unsigned i = 0; i < 32; i++) {
    for (unsigned j = 0; j < 16; ++j) {
      for (unsigned k = 0; k < 8; ++k) {
        printf(buffer[i * stride + j] & 1 << k ? "*" : " ");
      }
    }
    printf("|\n");
  }
}

int main() {  
  const int deviceNode = open("/dev/i2c-1", O_RDWR);
  const uint8_t addr = 0x3C;
  if (deviceNode < 0 || ioctl(deviceNode, I2C_SLAVE, addr) < 0) {
    printf("Could not open\n");
    abort();
  }

  writeBuffer(deviceNode, ssd1306_128x32_init_seq, sizeof(ssd1306_128x32_init_seq));

  const int stride = cairo_format_stride_for_width(CAIRO_FORMAT_A1, 128);
  assert(stride == 16);
  uint8_t data[512];
  cairo_surface_t *cairo_surface = cairo_image_surface_create_for_data(
    data, CAIRO_FORMAT_A1, 128, 32, stride
  );
  cairo_t *cr = cairo_create(cairo_surface);
  cairo_select_font_face(cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
  cairo_set_font_size(cr, 42);
  cairo_set_source_rgb(cr, 1, 1, 1);

  uint8_t buffer[513] = {SETSTARTLINE};
  while (1) {
    const time_t now = time(0);
    struct tm *tm = localtime(&now);
    char str[8] = {};
    snprintf(str, 6, "%02d%c%02d", tm->tm_hour, now % 2 ? ':' : ' ', tm->tm_min);

    memset(data, 0, stride * 32);
    cairo_move_to(cr, 0, 32);
    cairo_show_text(cr, str);
    debug(data, stride);

    for (unsigned i = 0; i < sizeof(buffer) - 1; ++i) {
      unsigned x = 127 - (i % 128);
      unsigned y = (i / 128) * 8;
      #define P(j) ((unsigned) !!(data[x / 8 + (y + j) * 16] & (1 << (7 - (x % 8)))) << j)
      buffer[i + 1] = P(0) | P(1) | P(2) | P(3) | P(4) | P(5) | P(6) | P(7);
      #undef P
    }
    writeBuffer(deviceNode, buffer, sizeof(buffer));
    usleep(1000000);
  }

  cairo_surface_destroy(cairo_surface);
  cairo_destroy(cr);
  close(deviceNode);
  return 0;
}
