// Display something on SSD1306 LCD in a Raspberry PI
// $ cc ssd1306.c && ./a.out
// Originally based on https://github.com/pive/rpi1306i2c/blob/master/rpi1306i2c.hpp

#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

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

uint8_t ssd1306_128x32_init_seq[] = {
  0x80, DISPLAYOFF,
  0x80, SETDISPLAYCLOCKDIV, 0x80, 0x80,
  0x80, SETMULTIPLEX, 0x80, 0x1F,
  0x80, SETDISPLAYOFFSET, 0x80, 0x00,
  0x80, SETSTARTLINE | 0x00,
  0x80, CHARGEPUMP, 0x80, 0x14,
  0x80, SEGREMAP,
  0x80, COMSCANDEC,
  0x80, SETCOMPINS, 0x80, 0x02,
  0x80, SETCONTRAST, 0x80, 0x1,//0x7F,
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

int main() {
  int deviceNode = open("/dev/i2c-1", O_RDWR);
  uint8_t addr = 0x3C;
  if (deviceNode < 0 || ioctl(deviceNode, I2C_SLAVE, addr) < 0) {
    printf("Could not open\n");
    abort();
  }

  writeBuffer(deviceNode, ssd1306_128x32_init_seq, sizeof(ssd1306_128x32_init_seq));

  uint8_t buffer[257] = {0x40};
  unsigned tick = 0;
  while (1) {
    ++tick;
    for (unsigned i = 0; i < sizeof(buffer) - 1; ++i) buffer[i + 1] = (i + tick) % 0x100;
    writeBuffer(deviceNode, buffer, sizeof(buffer));
    usleep(50000);
  }

  close(deviceNode);
  return 0;
}
