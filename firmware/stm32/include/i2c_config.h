#ifndef I2C_CONFIG_H
#define I2C_CONFIG_H

#include "main.h"

/*
 * Shared I2C1 bus used by both the OLED (SSD1306) and the LCD 16x2 —
 * see lib/hal_i2c_manager. PLACEHOLDER pins (I2C1 default remap:
 * PB6=SCL, PB7=SDA), confirm against a hardware pinout doc once it exists.
 */
#define I2C1_SCL_GPIO_Port GPIOB
#define I2C1_SCL_Pin GPIO_PIN_6
#define I2C1_SDA_GPIO_Port GPIOB
#define I2C1_SDA_Pin GPIO_PIN_7

extern I2C_HandleTypeDef hi2c1;
extern DMA_HandleTypeDef hdma_i2c1_tx;

void MX_I2C1_Init(void);

#endif /* I2C_CONFIG_H */
