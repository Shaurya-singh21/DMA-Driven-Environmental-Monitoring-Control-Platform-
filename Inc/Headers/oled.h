#ifndef __OLED_H
#define __OLED_H

#include "stm32f446xx.h"
#include <stdint.h>

#define OLED_W      128
#define OLED_H       64
#define OLED_PAGES    8
#define OLED_ADDR  0x78    //0x3C << 1
#define SYM_DEGREE  '\x01'

uint8_t oled_is_busy(void);

void oled_dma_complete(void);
void oled_cmd(uint8_t cmd);
void oled_init(void);
void oled_clear(void);
void oled_set_pixel(int x, int y, int on);
void oled_draw_char(int x, int page, char c);
void oled_print(int x, int page, const char *s);
void oled_flush(void);

#endif
