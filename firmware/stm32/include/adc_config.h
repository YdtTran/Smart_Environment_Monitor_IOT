#ifndef ADC_CONFIG_H
#define ADC_CONFIG_H

#include "main.h"

/*
 * Single ADC1 instance, channel reconfigured per read (see lib/adc_read).
 * PLACEHOLDER pins (PA0-PA3), confirm against a hardware pinout doc once
 * it exists.
 */
typedef enum
{
    ADC_CH_LDR = 0,             /* light sensor */
    ADC_CH_SAMPLING_RATE_POT,   /* sampling-rate config potentiometer */
    ADC_CH_LCD_SCROLL_POT,      /* LCD scroll-speed config potentiometer */
    ADC_CH_SERVO_FEEDBACK_POT,  /* servo feedback potentiometer */
    ADC_CH_COUNT
} adc_channel_id_t;

extern ADC_HandleTypeDef hadc1;

void MX_ADC1_Init(void);

/* Selects and configures the given logical channel for the next conversion. */
void ADC_Config_Channel(adc_channel_id_t id);

#endif /* ADC_CONFIG_H */
