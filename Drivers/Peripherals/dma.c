#include "stm32f446xx.h"
#include "dma.h"
#include "globals.h"
#include "oled.h"

extern volatile uint8_t flag;
extern uint16_t buffer[];

void dma_init(void){

	//dma1_channel1_stream6 for display
	DMA1_Stream6->CR &= ~(DMA_SxCR_EN);
	DMA1->HIFCR = DMA_HIFCR_CTCIF6 | DMA_HIFCR_CTEIF6 |
	              DMA_HIFCR_CDMEIF6 | DMA_HIFCR_CFEIF6 |
	              DMA_HIFCR_CHTIF6;
	DMA1_Stream6->PAR = (uint32_t)&I2C1->DR;
//	DMA1_Stream6->M0AR = (uint32_t)&display_buffer[0];
	DMA1_Stream6->CR &= ~((3U << DMA_SxCR_MSIZE_Pos) | (3U << DMA_SxCR_PSIZE_Pos) | (7U << DMA_SxCR_CHSEL_Pos));
	DMA1_Stream6->CR |= (1U << DMA_SxCR_CHSEL_Pos) | (1U << DMA_SxCR_TCIE_Pos) | (1U << DMA_SxCR_DIR_Pos) |
						(1U << DMA_SxCR_MINC_Pos);
	DMA1_Stream6->FCR &= ~(3U << DMA_SxFCR_DMDIS_Pos);
	NVIC_SetPriority(DMA1_Stream6_IRQn,4);
	NVIC_EnableIRQ(DMA1_Stream6_IRQn);


	//dma2_channel0_stream0(adc1) for ntc and ldr
	DMA2_Stream0->CR &= ~DMA_SxCR_EN;
	DMA2_Stream0->PAR = (uint32_t)&(ADC1->DR);
	DMA2_Stream0->M0AR = (uint32_t)&buffer[0];
	DMA2_Stream0->NDTR = DMA0_BUFFER_SIZE;
	DMA2_Stream0->CR &= ~((7U << DMA_SxCR_CHSEL_Pos) | (3U << DMA_SxCR_PL_Pos) | (3U << DMA_SxCR_DIR_Pos));
	DMA2_Stream0->CR |= (1U << DMA_SxCR_TCIE_Pos) | (1U << DMA_SxCR_TEIE_Pos) | (1U << DMA_SxCR_MSIZE_Pos) |
						(1U << DMA_SxCR_PSIZE_Pos);
	DMA2_Stream0->CR |= (1U << DMA_SxCR_MINC_Pos) | (1U << DMA_SxCR_CIRC_Pos);
	DMA2_Stream0->FCR &= ~(3U << DMA_SxFCR_DMDIS_Pos);
	NVIC_SetPriority(DMA2_Stream0_IRQn,2);
	NVIC_EnableIRQ(DMA2_Stream0_IRQn);
	DMA2->LIFCR |= DMA_LIFCR_CTCIF0 | DMA_LIFCR_CTEIF0 |
	               DMA_LIFCR_CDMEIF0 | DMA_LIFCR_CFEIF0 |
	               DMA_LIFCR_CHTIF0;
}


void DMA2_Stream0_IRQHandler(void){
	if(DMA2->LISR & DMA_LISR_TEIF0){
		DMA2_Stream0->CR &= ~(DMA_SxCR_EN);
		DMA2->LIFCR |= (1U << DMA_LIFCR_CTEIF0_Pos);
		DMA2_Stream0->CR |= (1U << DMA_SxCR_EN_Pos);
	}

	if(DMA2->LISR & DMA_LISR_TCIF0){
		GPIOC->BSRR = (GPIO_BSRR_BS8);
		DMA2->LIFCR |= (1U << DMA_LIFCR_CTCIF0_Pos);
		flag |= (DMA_PROCESS);
		if(!(flag & START_SYS)) GPIOC->BSRR = (GPIO_BSRR_BR8);
	}
}

void oled_dma_send(const uint8_t *buf, uint16_t len)
{
    DMA1_Stream6->CR &= ~DMA_SxCR_EN;
    while(DMA1_Stream6->CR & DMA_SxCR_EN);

    DMA1->HIFCR = DMA_HIFCR_CTCIF6  | DMA_HIFCR_CTEIF6  |
                  DMA_HIFCR_CDMEIF6 | DMA_HIFCR_CFEIF6  |
                  DMA_HIFCR_CHTIF6;

    DMA1_Stream6->M0AR = (uint32_t)buf;
    DMA1_Stream6->NDTR = len;

    I2C1->CR2 |= I2C_CR2_DMAEN;
    DMA1_Stream6->CR |= DMA_SxCR_EN;
    // returns immediately — ISR handles the rest
}

void DMA1_Stream6_IRQHandler(void){
    if(DMA1->HISR & DMA_HISR_TCIF6){
        DMA1->HIFCR  = DMA_HIFCR_CTCIF6;
        uint32_t t = 200000;
        while(!(I2C1->SR1 & I2C_SR1_BTF) && --t);
        I2C1->CR1   |= I2C_CR1_STOP;
        I2C1->CR2   &= ~I2C_CR2_DMAEN;
        oled_dma_complete();
    }
}
