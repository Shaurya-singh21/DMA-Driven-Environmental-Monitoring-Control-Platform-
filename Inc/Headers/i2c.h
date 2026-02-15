#ifndef __I2C_H
#define __I2C_H

#define I2C_TM  50000U
void i2c_gpio_init(void);
void i2c_send(uint8_t addr, const uint8_t *data, uint16_t len);

#endif
