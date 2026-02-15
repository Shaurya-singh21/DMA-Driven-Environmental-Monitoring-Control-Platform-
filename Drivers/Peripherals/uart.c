#include "stm32f446xx.h"
#include "uart.h"

void uart_init(void){
	//usart2 for data logging (pa2 for tx)
	GPIOA->MODER &= ~(GPIO_MODER_MODE2);
	GPIOA->MODER |= (2U << GPIO_MODER_MODE2_Pos);
	GPIOA->AFR[0] |= (7U << GPIO_AFRL_AFSEL2_Pos);
	GPIOA->OTYPER &= ~((1U << 2));
	GPIOA->OSPEEDR|=  (3U << GPIO_OSPEEDR_OSPEED2_Pos);

	//usart registers
	USART2->BRR = UART2_BAUD_RATE;
	USART2->CR1 |= (1U << USART_CR1_RXNEIE_Pos) | (1U << USART_CR1_TE_Pos);
	NVIC_EnableIRQ(USART2_IRQn);
	USART2->CR1 |= (1U << USART_CR1_UE_Pos);
}

volatile uint8_t uart_busy = 0;
volatile char*  word;


void send(char* sentence){
	if(uart_busy) return;
	uart_busy = 1;
	word = (char*)sentence;
	USART2->CR1 |= (1U << USART_CR1_TXEIE_Pos);
}

void USART2_IRQHandler(void){
	//transmit
	if((USART2->SR & (USART_SR_TXE)) && (USART2->CR1 & (USART_CR1_TXEIE))){
		if(*word == '\0'){
//			GPIOC->BSRR = (GPIO_BSRR_BR8);
			USART2->CR1 &= ~(1U << USART_CR1_TXEIE_Pos);
			uart_busy = 0;
		}else{
			USART2->DR = *word++;
		}
	}
}

