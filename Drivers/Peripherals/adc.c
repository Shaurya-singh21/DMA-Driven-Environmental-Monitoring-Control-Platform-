#include "stm32f446xx.h"

void adc_init(void){
	//adc1_channel_0 for ntc thermistor and temp logging pa0
	GPIOA->MODER &= ~(3U <<GPIO_MODER_MODE0_Pos);
	GPIOA->MODER |= (3U << GPIO_MODER_MODE0_Pos);


	//adc1_channel_1 for ldr pa1
	GPIOA->MODER &= ~(3U << GPIO_MODER_MODE1_Pos);
	GPIOA->MODER |= (3U << GPIO_MODER_MODE1_Pos);

	ADC1->CR1 &= ~(3U << ADC_CR1_RES_Pos);//12 bit res
	ADC1->CR1 |= (1U << ADC_CR1_SCAN_Pos);//scan mode
	ADC1->CR2 &= ~(ADC_CR2_CONT);//one shot
	ADC1->CR2 |= (1U << ADC_CR2_DMA_Pos) | (1U << ADC_CR2_DDS_Pos); //dma enable and dma disable select
	ADC1->SMPR2 |= (7U << ADC_SMPR2_SMP0_Pos) | (7U << ADC_SMPR2_SMP1_Pos);//480cycle st for ch0 and ch1
	ADC1->SQR1 &= ~(15U << ADC_SQR1_L_Pos);
	ADC1->SQR1 |= (1U << ADC_SQR1_L_Pos);
	ADC1->SQR3 &= ~(31U << ADC_SQR3_SQ1_Pos);
	ADC1->SQR3 |= (0U << ADC_SQR3_SQ1_Pos);  // First conversion: channel 0
	ADC1->SQR3 &= ~(31U << ADC_SQR3_SQ2_Pos);
	ADC1->SQR3 |= (1U << ADC_SQR3_SQ2_Pos);  // Second conversion: channel 1

	//automated checking by timer_3 every 1000ms
//	ADC1->CR2 &= ~ADC_CR2_EXTSEL;
//	ADC1->CR2 &= ~ADC_CR2_EXTEN;
//	ADC1->CR2 |= (1U << ADC_CR2_EXTEN_Pos) | (7U << ADC_CR2_EXTSEL_Pos);
	ADC1->CR2 &= ~ADC_CR2_EXTSEL;
	ADC1->CR2 &= ~ADC_CR2_EXTEN;
	ADC1->CR2 |= (1U << ADC_CR2_EXTEN_Pos)  // trigger on rising edge
	           | (8U << ADC_CR2_EXTSEL_Pos); // TIM3 TRGO
	ADC1->CR2 |= (1U << ADC_CR2_ADON_Pos);
}


