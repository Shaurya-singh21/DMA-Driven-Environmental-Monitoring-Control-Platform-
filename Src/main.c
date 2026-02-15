#include "stm32f446xx.h"
#include "stdio.h"
#include "clock.h"
#include "timer.h"
#include "uart.h"
#include "dma.h"
#include "adc.h"
#include "gpio.h"
#include "i2c.h"
#include "globals.h"
#include "math.h"
#include "string.h"
#include "oled.h"
sys_info dp = { 0 };

volatile uint16_t pwm_target = 0;
volatile uint8_t low_temp_read = 0;
volatile uint8_t high_temp_read = 0;
volatile uint8_t flag = 0;
//dma variables
uint16_t buffer[DMA0_BUFFER_SIZE];
float ldr;
uint8_t proximity;
//uart varaible
char uart_buffer[100];
extern volatile uint8_t uart_busy;
volatile uint8_t dma_busy = 0;

void stop_cooling(void) {
	flag &= ~(COOLING_PROCESS);
	//blink off
	TIM2->CR1 &= ~(TIM_CR1_CEN);
	//vent close
	TIM4->DIER |= (TIM_DIER_UIE);
	pwm_target = 2000;
	dp.fan = 0;
	high_temp_read = 0;
}
void stop_heat(void) {
	flag &= ~(HEATING_PROCESS);
	//blink off
	TIM2->CR1 &= ~(TIM_CR1_CEN);
	//vent close
	TIM4->DIER |= (TIM_DIER_UIE);
	pwm_target = 2000;
}
void check_temp(void) {
	float vout = ((buffer[0] * 3.3f) / (4095.0f));
	float rntc = Rfix * (3.3f / vout - 1.0f);
	float lnR = logf(rntc);
	float inv_t = A + (B * lnR) + (C * lnR * lnR * lnR);
	float tempK = 1.0f / inv_t;
	dp.temp = (tempK - 273.15f);   // convert to Celsius
	if (dp.temp > optimum_temp_high) {
		++high_temp_read;
		if (!(flag & COOLING_PROCESS) && high_temp_read > 4) {
			flag |= (COOLING_PROCESS);
			dp.fan = 1;
			high_temp_read = 0;
		}
	} else if (dp.temp < optimum_temp_low) {
		++low_temp_read;
		if (!(flag & HEATING_PROCESS) && low_temp_read > 4) {
			flag |= HEATING_PROCESS;
			low_temp_read = 0;
		}
	} else {
		high_temp_read = 0;
		low_temp_read = 0;
		if (flag & COOLING_PROCESS) {
			stop_cooling();
		}
		if (flag & HEATING_PROCESS)
			stop_heat();
	}
}

void check_ldr_ir_proximity() {
	float vout = ((buffer[1] * 3.3f) / (4095.0f));
	float rldr = Rfix * (3.3f / vout - 1.0f);
	float lux = pow(500000.0f / rldr, 1.428f);
	ldr = lux;
	proximity = GPIOF->IDR & (GPIO_IDR_ID6);
	if (ldr > LDR_Threshold && !proximity)
		dp.door = 1;
	if (ldr < LDR_Threshold && dp.door == 1)
		dp.door = 0;
}
void update_display(void) {
	if (oled_is_busy())
		return;
	char line[22];
	snprintf(line, sizeof(line), "    Temp: %.1f C    ", dp.temp);
	oled_print(0, 2, line);
	snprintf(line, sizeof(line), "    Door: %s", dp.door ? "OPEN" : "CLOSED");
	oled_print(0, 4, line);
	snprintf(line, sizeof(line), "    Fan: %s", dp.fan ? "ON" : "OFF");
	oled_print(0, 5, line);
	oled_print(0, 6, "                     ");
	oled_flush();
}
void process_dma_data(void) {
	check_temp();
	check_ldr_ir_proximity();
	while (uart_busy)
		;
	memset(uart_buffer, 0, sizeof(uart_buffer));
	snprintf(uart_buffer, sizeof(uart_buffer),
			"Temp:%1f, LDR:%1f ,NTC_RAW:%u, LDR_RAW:%u,DOOR: %u ,cnt:%u \r\n",
			dp.temp, ldr, buffer[0], buffer[1], dp.door, high_temp_read);
	send((char*) uart_buffer);
	update_display();
}

void welcome_message(void) {
	oled_clear();
	oled_print(0, 0, " PARAMETERS MONITOR  ");
	oled_print(0, 1, "---------------------");
	oled_print(0, 2, "       WELCOME       ");
	oled_print(0, 4, "Press Button To Start");
	oled_print(0, 6, "          ->         ");
	oled_flush();
}

uint8_t sys_initialized = 0;
void sys_stop(void) {
	//tim3 stop
	TIM3->CR1 &= ~(TIM_CR1_CEN);
	TIM3->CNT = 0;
	//servo back to initial
	TIM4->DIER |= (TIM_DIER_UIE);
	pwm_target = 2000;
	TIM2->CCR1 = 0;
	//dma_stop
	DMA2_Stream0->CR &= ~DMA_SxCR_EN;
	while (DMA2_Stream0->CR & DMA_SxCR_EN);
	DMA2->LIFCR = (DMA_LIFCR_CTCIF0 | DMA_LIFCR_CHTIF0 | DMA_LIFCR_CTEIF0
			| DMA_LIFCR_CDMEIF0 | DMA_LIFCR_CFEIF0);

	welcome_message();
	GPIOC->BSRR = (GPIO_BSRR_BR7);
	sys_initialized = 0;
}

void start_cooling(void) {
	//blink led in timer
	TIM2->CR1 |= (TIM_CR1_CEN);
	TIM2->CCR1 = 1999;
	//vent open
	TIM4->DIER |= (TIM_DIER_UIE);
	pwm_target = 1000;
	dp.fan = 1;
}

void start_heating(void) {
	//blink led in timer
	TIM2->CR1 |= (TIM_CR1_CEN);
	TIM2->CCR1 = 1999;
	//vent open 45 degree
	TIM4->DIER |= (TIM_DIER_UIE);
	pwm_target = 1500;
}

int main(void) {
	SCB->CPACR |= ((3UL << 10 * 2) | (3UL << 11 * 2));
	config_clock();
	gpio_init();
	timer_init();
	uart_init();
	i2c_gpio_init();
	dma_init();
	oled_init();
	adc_init();
	welcome_message();
	for (;;) {
		if (flag & START_SYS) {
			if (!sys_initialized) {
				GPIOC->BSRR = (GPIO_BSRR_BS7);
				sys_initialized = 1;
				oled_print(0, 4, "                     ");
				oled_print(0, 5, "  SENDING DATA ....  ");
				oled_print(0, 6, "                     ");
				oled_flush();
				TIM3->CR1 |= (TIM_CR1_CEN);
				DMA2_Stream0->CR |= DMA_SxCR_EN;
				TIM6->CR1 |= (TIM_CR1_CEN);
			}
			if (flag & DMA_PROCESS) {
				process_dma_data();
				flag &= ~(DMA_PROCESS);
			}
			if (flag & COOLING_PROCESS) {
				start_cooling();
				flag &= ~(COOLING_PROCESS);
			}
			if (flag & HEATING_PROCESS) {
				start_heating();
				flag &= ~(HEATING_PROCESS);
			}
		} else {
			if (sys_initialized) {
				sys_stop();
			}
		}
	}
}
