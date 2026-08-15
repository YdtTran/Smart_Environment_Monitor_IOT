#include "gpio_config.h"

void GPIO_Config_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* TODO */
    /* Builtin LED config */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = WARNING_LED_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(WARNING_LED_GPIO_Port, &GPIO_InitStruct);

    /* I2C pin config */
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7; /* PB6 = I2C1_SCL, PB7 = I2C1_SDA */
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* DHT11_Pin (PB15) tu cau hinh EXTI + NVIC rieng trong DHT11_Init(), khong
     * dung chung EXTI0 nhu ban test truoc do (PA0/EXTI0 khong khop voi PB15). */
}
