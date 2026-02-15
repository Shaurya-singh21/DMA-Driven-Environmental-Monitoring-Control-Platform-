#include "stm32f446xx.h"
#include "globals.h"

extern volatile uint8_t flag;
void gpio_init(void) {

	GPIOC->MODER &= ~(3U << GPIO_MODER_MODE6_Pos); //pc6 for ir proximity

	GPIOC->MODER &= ~(3U << GPIO_MODER_MODE7_Pos);
	GPIOC->MODER |= (1U << GPIO_MODER_MODE7_Pos); // PC7 for container light

	GPIOF->MODER &= ~(3U << GPIO_MODER_MODE4_Pos); // PF4 for motor direc
	GPIOF->MODER |= (1U << GPIO_MODER_MODE4_Pos);
	GPIOF->OTYPER &= ~(GPIO_OTYPER_OT4);

	GPIOC->MODER &= ~(3U << GPIO_MODER_MODE13_Pos);
	GPIOC->PUPDR &= ~(3U << GPIO_PUPDR_PUPD13_Pos); // PC13 for starting system

	SYSCFG->EXTICR[3] &= ~(0xF << SYSCFG_EXTICR4_EXTI13_Pos);
	SYSCFG->EXTICR[3] |= (2U << SYSCFG_EXTICR4_EXTI13_Pos);

	EXTI->IMR |= EXTI_IMR_MR13;
	EXTI->FTSR |= EXTI_FTSR_TR13;
	EXTI->PR = EXTI_PR_PR13;

	NVIC_SetPriority(EXTI15_10_IRQn, 1);
	NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void EXTI15_10_IRQHandler(void) {
	EXTI->PR = EXTI_PR_PR13;
	if (!(GPIOC->IDR & GPIO_IDR_ID13)) {
			flag ^= START_SYS;
	}
}

