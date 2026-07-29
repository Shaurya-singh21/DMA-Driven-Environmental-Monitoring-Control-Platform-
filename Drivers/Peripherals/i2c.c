#include "stm32f446xx.h"
#include "globals.h"
#include "string.h"
#include "i2c.h"
#include "oled.h"
extern volatile uint8_t flag;


static void delay_short(volatile uint32_t n){ while(n--); }

void delay_ms(volatile uint32_t ms){
    while(ms--)
        for(volatile uint32_t i = 0; i < 1600; i++);
}

/* Bit-bang SCL to release a slave holding SDA low after an interrupted transfer.
 * Must be called BEFORE pins are switched to AF mode (i2c_gpio_init). */
void i2c_bus_recover(void) {
    /* Temporarily set PB6(SCL), PB7(SDA) as GPIO open-drain outputs */
    GPIOB->MODER  &= ~((3U << GPIO_MODER_MODE6_Pos) | (3U << GPIO_MODER_MODE7_Pos));
    GPIOB->MODER  |=  (1U << GPIO_MODER_MODE6_Pos)  | (1U << GPIO_MODER_MODE7_Pos);
    GPIOB->OTYPER |=  (1U << 6) | (1U << 7);   /* open-drain */

    GPIOB->BSRR = (1U << 7);  /* release SDA high */
    delay_short(2000);

    /* Clock SCL up to 9 times until SDA is released */
    for (int i = 0; i < 9; i++) {
        GPIOB->BSRR = (1U << 22);  /* SCL low  (BR6 = bit 22) */
        delay_short(2000);
        GPIOB->BSRR = (1U << 6);   /* SCL high */
        delay_short(2000);
        if (GPIOB->IDR & (1U << 7)) break;  /* SDA released */
    }

    /* Generate manual STOP: SDA low→high while SCL is high */
    GPIOB->BSRR = (1U << 23);  /* SDA low  (BR7 = bit 23) */
    delay_short(2000);
    GPIOB->BSRR = (1U << 6);   /* SCL high */
    delay_short(2000);
    GPIOB->BSRR = (1U << 7);   /* SDA high → STOP condition */
    delay_short(2000);
}

void i2c_reset(void){
    I2C1->CR1 |=  I2C_CR1_SWRST;
    delay_ms(10);
    I2C1->CR1 &= ~I2C_CR1_SWRST;
    delay_ms(5);
    I2C1->CR2 = (16U << I2C_CR2_FREQ_Pos);
    I2C1->CCR   = 80;
    I2C1->TRISE = 17;
    I2C1->CR1  |= I2C_CR1_PE;
    uint32_t t = 50000;
    uint32_t timeout = 10000;
    while (((GPIOB->IDR & (1U << 6)) == 0 || (GPIOB->IDR & (1U << 7)) == 0) && --timeout) {
           // Wait for bus idle
       }
    while((I2C1->SR2 & I2C_SR2_BUSY) && --t);
}

void i2c_gpio_init(void){
    //PB6=SCL, PB7=SDA
    GPIOB->MODER   &= ~((3U << GPIO_MODER_MODE6_Pos) | (3U << GPIO_MODER_MODE7_Pos));
    GPIOB->MODER   |=  (2U << GPIO_MODER_MODE6_Pos)  | (2U << GPIO_MODER_MODE7_Pos);
    GPIOB->OTYPER  |=  (1U << 6) | (1U << 7);
    GPIOB->OSPEEDR |=  (3U << GPIO_OSPEEDR_OSPEED6_Pos) | (3U << GPIO_OSPEEDR_OSPEED7_Pos);
    GPIOB->PUPDR   &= ~((3U << GPIO_PUPDR_PUPD6_Pos) | (3U << GPIO_PUPDR_PUPD7_Pos));
    GPIOB->AFR[0]  &= ~((0xFU << GPIO_AFRL_AFSEL6_Pos) | (0xFU << GPIO_AFRL_AFSEL7_Pos));
    GPIOB->AFR[0]  |=  (4U << GPIO_AFRL_AFSEL6_Pos) | (4U << GPIO_AFRL_AFSEL7_Pos);
    i2c_reset();
}

//Blocking I2C send
void i2c_send(uint8_t addr, const uint8_t *data, uint16_t len)
{
    uint32_t t;

    if (I2C1->SR2 & I2C_SR2_BUSY) {
        i2c_reset();
        for (volatile int i = 0; i < 1600; i++);
    }

    I2C1->CR1 |= I2C_CR1_START;
    t = I2C_TM; while (!(I2C1->SR1 & I2C_SR1_SB)   && --t);
    if (!t) { I2C1->CR1 |= I2C_CR1_STOP; return; }

    I2C1->DR = addr;
    t = I2C_TM; while (!(I2C1->SR1 & I2C_SR1_ADDR) && --t);
    if (!t) { I2C1->CR1 |= I2C_CR1_STOP; return; }
    (void)I2C1->SR1; (void)I2C1->SR2; 

    for (uint16_t i = 0; i < len; i++) {
        t = I2C_TM; while (!(I2C1->SR1 & I2C_SR1_TXE) && --t);
        if (!t) { I2C1->CR1 |= I2C_CR1_STOP; return; }
        I2C1->DR = data[i];
    }

    t = I2C_TM * 4; while (!(I2C1->SR1 & I2C_SR1_BTF) && --t);

    I2C1->CR1 |= I2C_CR1_STOP;
    for (volatile int i = 0; i < 1600; i++);
}





