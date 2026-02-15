#include "stm32f446xx.h"
#include "globals.h"
#include "string.h"
#include "i2c.h"
#include "oled.h"
extern volatile uint8_t flag;



/* ── Delay ── */


//void i2c_init(void){
//	//i2c1 for display (pb6,pb7)
//	GPIOB->MODER &= ~((3U << GPIO_MODER_MODE6_Pos) | (3U << GPIO_MODER_MODE7_Pos));
//	GPIOB->MODER |= (2U << GPIO_MODER_MODE6_Pos) | ((2U << GPIO_MODER_MODE7_Pos));
//	GPIOB->OTYPER |= (1U << GPIO_OTYPER_OT6_Pos) | (1U << GPIO_OTYPER_OT7_Pos);
//
//	GPIOB->AFR[0] |= (4U << GPIO_AFRL_AFSEL6_Pos) ;
//
//
//	I2C1->CR2 |= (16U << I2C_CR2_FREQ_Pos);
//	I2C1->CCR &= ~(I2C_CCR_FS);
//	I2C1->CCR |= (80 & 0xFFF);
//	I2C1->TRISE = 17;
//	I2C1->CR2 |= (1U << I2C_CR2_DMAEN_Pos);
//	I2C1->CR1 |= (1U << I2C_CR1_PE_Pos);
//}
//
//void update_disp(char* info){
//	perm = 0;
//	I2C1->CR1 |= (1U << I2C_CR1_START_Pos);
//	while(!(I2C1->SR1 & I2C_SR1_SB));
//	I2C1->DR = 0x3C << 1;
//	while(!(I2C1->SR1 & I2C_SR1_ADDR));
//	(void)I2C1->SR1; (void)I2C1->SR2;
//
//
//	DMA1_Stream6->CR &= ~(DMA_SxCR_EN);
//	DMA1->HIFCR |= DMA_HIFCR_CTCIF6;
//	DMA1_Stream6->NDTR = 1024;
//
//
//	DMA1_Stream6->CR |=  DMA_SxCR_EN;
//}
//

static void delay_ms(volatile uint32_t ms){
    while(ms--)
        for(volatile uint32_t i = 0; i < 1600; i++);
}

/* ── I2C reset ── */
static void i2c_reset(void){
    I2C1->CR1 |=  I2C_CR1_SWRST;
    delay_ms(10);
    I2C1->CR1 &= ~I2C_CR1_SWRST;
    delay_ms(5);
    I2C1->CR2 = (16U << I2C_CR2_FREQ_Pos);
    I2C1->CCR   = 80;
    I2C1->TRISE = 17;
    I2C1->CR1  |= I2C_CR1_PE;
    uint32_t t = 50000;
    while((I2C1->SR2 & I2C_SR2_BUSY) && --t);
}

/* ── i2c_init — call once from main ── */
void i2c_gpio_init(void){
    //PB6=SCL, PB7=SDA:
    GPIOB->MODER   &= ~((3U << GPIO_MODER_MODE6_Pos) | (3U << GPIO_MODER_MODE7_Pos));
    GPIOB->MODER   |=  (2U << GPIO_MODER_MODE6_Pos)  | (2U << GPIO_MODER_MODE7_Pos);
    GPIOB->OTYPER  |=  (1U << 6) | (1U << 7);
    GPIOB->OSPEEDR |=  (3U << GPIO_OSPEEDR_OSPEED6_Pos) | (3U << GPIO_OSPEEDR_OSPEED7_Pos);
    GPIOB->PUPDR   &= ~((3U << GPIO_PUPDR_PUPD6_Pos) | (3U << GPIO_PUPDR_PUPD7_Pos));
    GPIOB->AFR[0]  &= ~((0xFU << GPIO_AFRL_AFSEL6_Pos) | (0xFU << GPIO_AFRL_AFSEL7_Pos));
    GPIOB->AFR[0]  |=  (4U << GPIO_AFRL_AFSEL6_Pos) | (4U << GPIO_AFRL_AFSEL7_Pos);
    i2c_reset();
}

/* ── Blocking I2C send ── */
void i2c_send(uint8_t addr, const uint8_t *data, uint16_t len)
{
    uint32_t t;

    /* Recover if bus is stuck busy */
    if (I2C1->SR2 & I2C_SR2_BUSY) {
        i2c_reset();
        for (volatile int i = 0; i < 1600; i++);
    }

    /* START */
    I2C1->CR1 |= I2C_CR1_START;
    t = I2C_TM; while (!(I2C1->SR1 & I2C_SR1_SB)   && --t);
    if (!t) { I2C1->CR1 |= I2C_CR1_STOP; return; }

    /* Address byte — write direction (LSB = 0, already set by shift) */
    I2C1->DR = addr;
    t = I2C_TM; while (!(I2C1->SR1 & I2C_SR1_ADDR) && --t);
    if (!t) { I2C1->CR1 |= I2C_CR1_STOP; return; }
    (void)I2C1->SR1; (void)I2C1->SR2;   /* reading SR1+SR2 clears ADDR */

    /* Data bytes */
    for (uint16_t i = 0; i < len; i++) {
        t = I2C_TM; while (!(I2C1->SR1 & I2C_SR1_TXE) && --t);
        if (!t) { I2C1->CR1 |= I2C_CR1_STOP; return; }
        I2C1->DR = data[i];
    }

    /* Wait for last byte fully shifted out */
    t = I2C_TM * 4; while (!(I2C1->SR1 & I2C_SR1_BTF) && --t);

    /* STOP */
    I2C1->CR1 |= I2C_CR1_STOP;
    for (volatile int i = 0; i < 1600; i++);
}





