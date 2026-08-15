#ifndef GPIO_CONFIG_H
#define GPIO_CONFIG_H

#include "main.h"

/*
 * PLACEHOLDER pin assignment — not yet confirmed against a hardware pinout
 * doc (docs/hardware/stm32-pinout.md doesn't exist yet). Update these
 * defines once the real wiring is decided; nothing outside this header
 * should hardcode the pin/port directly.
 */
#define WARNING_LED_GPIO_Port GPIOC
#define WARNING_LED_Pin GPIO_PIN_13

#define INTERUPT_Pin GPIO_PIN_0
#define INTERUPT_GPIO_Port GPIOA

#define DHT11_Pin GPIO_PIN_15
#define DHT11_GPIO_Port GPIOB

void GPIO_Config_Init(void);

#endif /* GPIO_CONFIG_H */
