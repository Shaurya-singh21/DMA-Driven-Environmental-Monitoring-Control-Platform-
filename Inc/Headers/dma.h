#ifndef __DMA_H
#define __DMA_H
#define DMA0_BUFFER_SIZE 2U

void dma_init(void);
void oled_dma_send(const uint8_t *buf, uint16_t len);
#endif
