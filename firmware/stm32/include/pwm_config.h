#ifndef PWM_CONFIG_H
#define PWM_CONFIG_H

#include "main.h"

/*
 * TIM1 CH1 drives the 360-degree servo (see lib/decision_block).
 * PLACEHOLDER pin (PA8), confirm against a hardware pinout doc once it
 * exists.
 */
#define SERVO_PWM_GPIO_Port GPIOA
#define SERVO_PWM_Pin       GPIO_PIN_8
#define SERVO_PWM_CHANNEL   TIM_CHANNEL_1

extern TIM_HandleTypeDef htim_servo;

void MX_ServoPWM_Init(void);

#endif /* PWM_CONFIG_H */
