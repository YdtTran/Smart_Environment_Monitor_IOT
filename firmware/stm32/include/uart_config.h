#ifndef UART_CONFIG_H
#define UART_CONFIG_H

#include "main.h"

/*
 * USART1 carries the STM32 -> ESP32 data frame (see lib/uart_frame).
 * PLACEHOLDER pins (PA9=TX, PA10=RX), confirm against a hardware pinout
 * doc once it exists.
 */
#define UART_ESP32_TX_GPIO_Port GPIOA
#define UART_ESP32_TX_Pin       GPIO_PIN_9
#define UART_ESP32_RX_GPIO_Port GPIOA
#define UART_ESP32_RX_Pin       GPIO_PIN_10
#define UART_ESP32_BAUDRATE     115200U

extern UART_HandleTypeDef huart_esp32;

void MX_UART_ESP32_Init(void);

#endif /* UART_CONFIG_H */
