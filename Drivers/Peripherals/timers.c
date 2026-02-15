#include "stm32f446xx.h"
#include "globals.h"

extern volatile uint16_t pwm_target;
extern volatile uint8_t flag;
volatile uint8_t disp_trigger = 0;
void timer_init(void) {
	//timer tim6 for opening vent (s)
	TIM6->PSC = 1599;
	TIM6->ARR = 199;
	TIM6->EGR |= (1 << TIM_EGR_UG_Pos);
	TIM6->SR = 0;
	TIM6->DIER |= (1U << TIM_DIER_UIE_Pos);
	//tim3_trgo for adc (we are doing this ccmr1 & ccer coz tim3 has 4 channels)
	TIM3->PSC = 15999;
	TIM3->ARR = 999;
	TIM3->EGR |= TIM_EGR_UG;
	TIM3->SR = 0;
	TIM3->CR2 &= ~TIM_CR2_MMS;
	TIM3->CR2 |= (2U << TIM_CR2_MMS_Pos);  // TRGO on update

	//tim4_ch3_pwm for servo (50Hz standard) (pb8)
	GPIOB->MODER &= ~(3U << GPIO_MODER_MODE8_Pos);
	GPIOB->MODER |= (2U << GPIO_MODER_MODE8_Pos);
	GPIOB->AFR[1] |= (2U << GPIO_AFRH_AFSEL8_Pos);

	TIM4->PSC = 15;
	TIM4->ARR = 19999;
	TIM4->EGR |= (1U << TIM_EGR_UG_Pos);
	TIM4->SR = 0;
	TIM4->CR1 |= (1U << TIM_CR1_ARPE_Pos);
	TIM4->CCMR2 &= ~(3U << TIM_CCMR2_CC3S_Pos);
	TIM4->CCMR2 |= (1U << TIM_CCMR2_OC3PE_Pos) | (6U << TIM_CCMR2_OC3M_Pos);
	TIM4->CCER |= (1U << TIM_CCER_CC3E_Pos);
	TIM4->CCR3 = 2000;
	TIM4->CR1 |= (TIM_CR1_CEN);
	NVIC_SetPriority(TIM4_IRQn,5);
	NVIC_EnableIRQ(TIM4_IRQn);
	//tim2_ch1 for led blink when temp rises

	GPIOA->MODER &= ~(3U << GPIO_MODER_MODE5_Pos);
	GPIOA->MODER |= (2U << GPIO_MODER_MODE5_Pos);
	GPIOA->AFR[0] &= ~(15U << GPIO_AFRL_AFSEL5_Pos);
	GPIOA->AFR[0] |= (1U << GPIO_AFRL_AFSEL5_Pos);

	TIM2->PSC = 799;
	TIM2->ARR = 3999;
	TIM2->EGR |= (1U << TIM_EGR_UG_Pos);
	TIM2->SR = 0;
	TIM2->CCMR1 &= ~(3U << TIM_CCMR1_CC1S_Pos);
	TIM2->CCMR1 |= (6U << TIM_CCMR1_OC1M_Pos) | (1U << TIM_CCMR1_OC1PE_Pos);
	TIM2->CCER |= (1U << TIM_CCER_CC1E_Pos);
	TIM2->CCR1 = 1999;

	//tim1_ch2(pa9) for dc motor speed
	TIM1->PSC = 7;
	TIM1->ARR = 999;
	TIM1->EGR |= (1U << TIM_EGR_UG_Pos);
	TIM1->SR = 0;
	TIM1->CCMR1	|= (6U << TIM_CCMR1_OC2M_Pos) | (1U << TIM_CCMR1_OC2PE_Pos);
	TIM1->CCER |= (TIM_CCER_CC2E);
	TIM1->CCR2 = 0;
}

void TIM4_IRQHandler(void){
	if(TIM4->SR & TIM_SR_UIF){
		TIM4->SR &= ~(TIM_SR_UIF);
			if(TIM4->CCR3 > pwm_target) TIM4->CCR3 -= 20;
			else if(TIM4->CCR3 < pwm_target) TIM4->CCR3 += 20;
			else {
				TIM4->DIER &= ~(TIM_DIER_UIE);
				pwm_target = 0;
			}
	}
}

