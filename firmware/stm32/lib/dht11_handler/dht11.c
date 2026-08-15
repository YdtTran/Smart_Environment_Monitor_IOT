/**
 * @file    dht11.c
 * @brief   DHT11 driver - skeleton. Xem kien truc 2 lop trong dht11.h.
 */

#include "dht11.h"

uint32_t GetTimerClockFreq(TIM_TypeDef *instance)
{
    RCC_ClkInitTypeDef clkconfig;
    uint32_t pFLatency;
    HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);

    if (instance == TIM1)
    {
        uint32_t pclk2 = HAL_RCC_GetPCLK2Freq();
        return (clkconfig.APB2CLKDivider == RCC_HCLK_DIV1) ? pclk2 : pclk2 * 2;
    }
    else // TIM2, TIM3, TIM4 đều trên APB1
    {
        uint32_t pclk1 = HAL_RCC_GetPCLK1Freq();
        return (clkconfig.APB1CLKDivider == RCC_HCLK_DIV1) ? pclk1 : pclk1 * 2;
    }
}

static void EnableTimerClock(TIM_TypeDef *instance)
{
    if (instance == TIM1)
    {
        __HAL_RCC_TIM1_CLK_ENABLE();
    }
    else if (instance == TIM2)
    {
        __HAL_RCC_TIM2_CLK_ENABLE();
    }
    else if (instance == TIM3)
    {
        __HAL_RCC_TIM3_CLK_ENABLE();
    }
    else if (instance == TIM4)
    {
        __HAL_RCC_TIM4_CLK_ENABLE();
    }
}

static IRQn_Type GetExtiIRQn(uint16_t pin)
{
    switch (pin)
    {
    case GPIO_PIN_0:
        return EXTI0_IRQn;
    case GPIO_PIN_1:
        return EXTI1_IRQn;
    case GPIO_PIN_2:
        return EXTI2_IRQn;
    case GPIO_PIN_3:
        return EXTI3_IRQn;
    case GPIO_PIN_4:
        return EXTI4_IRQn;
    default:
        return (pin < GPIO_PIN_10) ? EXTI9_5_IRQn : EXTI15_10_IRQn;
    }
}

void InitTimer_1us(TIM_HandleTypeDef *htim)
{
    EnableTimerClock(htim->Instance);
    uint32_t timClk = GetTimerClockFreq(htim->Instance);

    htim->Init.Prescaler = (timClk / 1000000) - 1;
    htim->Init.CounterMode = TIM_COUNTERMODE_UP;
    htim->Init.Period = 0xFFFF; // F103: TẤT CẢ timer đều 16-bit
    htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    HAL_TIM_Base_Init(htim);

    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    HAL_TIM_ConfigClockSource(htim, &sClockSourceConfig);

    HAL_TIM_Base_Start(htim);
}

void GPIO_SetPinOutput(GPIO_TypeDef *port, uint16_t pin)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = pin;
    /* Open-drain, khong phai push-pull: DHT11 la bus 1-day, chi mot ben duoc
     * chu dong keo LOW tai 1 thoi diem. Neu dung push-pull va ep HIGH ngay
     * luc sensor van con dang keo LOW (VD: timeout giua chung o RECEIVING),
     * hai driver se dau do truc tiep 3.3V-GND, lau ngay lam sensor liet han. */
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(port, &GPIO_InitStruct);
    /* HAL khong tu clear EXTI mask khi chuyen tu IT_FALLING sang OUTPUT - tu tay
     * tat de tranh MCU tu kich hoat ngat khi chinh no keo line xuong LOW. */
    EXTI->IMR &= ~(uint32_t)pin;
    EXTI->PR = pin;
}
void GPIO_SetPinInput(GPIO_TypeDef *port, uint16_t pin)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(port, &GPIO_InitStruct);
    EXTI->PR = pin; /* xoa pending flag cu truoc khi bat dau nhan tin hieu that */
}

DHT11_Status_t DHT11_Init(DHT11_Handle_t *handle,
                          GPIO_TypeDef *port, uint16_t pin,
                          TIM_HandleTypeDef *htim, uint32_t channel,
                          uint32_t sample_period_ms, uint32_t timeout_ms)
{
    /* TODO: luu cau hinh vao handle, dat state = IDLE, cau hinh GPIO ban dau (output, keo cao). */
    handle->port = port;
    handle->pin = pin;
    handle->htim = htim;
    handle->timer_channel = channel;
    handle->sample_period_ms = sample_period_ms;
    handle->timeout_ms = timeout_ms;
    handle->state = DHT11_STATE_IDLE;
    GPIO_SetPinOutput(port, pin);
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
    InitTimer_1us(htim);

    IRQn_Type irqn = GetExtiIRQn(pin);
    HAL_NVIC_SetPriority(irqn, 5, 0);
    HAL_NVIC_EnableIRQ(irqn);

    return DHT11_OK;
}

void DHT11_Read(DHT11_Handle_t *handle)
{
    /* TODO: kiem tra elapsed >= sample_period_ms de trigger (IDLE -> START_SIGNAL).
     *       kiem tra timeout dua tren entry_tick (=> ERROR).
     *       kiem tra state == DONE/ERROR de xu ly ket qua, chuyen ve IDLE.
     *       Non-blocking - khong goi HAL_Delay().
     */
    uint32_t current_tick = HAL_GetTick();
    switch (handle->state)
    {
    case DHT11_STATE_IDLE:
        if (current_tick - handle->last_task_run >= handle->sample_period_ms)
        {
            /* Trigger doc moi: keo line xuong thap, chuyen sang START_SIGNAL. */
            GPIO_SetPinOutput(handle->port, handle->pin);
            HAL_GPIO_WritePin(handle->port, handle->pin, GPIO_PIN_RESET);
            handle->entry_tick = current_tick;
            handle->state = DHT11_STATE_START_SIGNAL;
            __HAL_TIM_SET_COUNTER(handle->htim, 0); // Timer CNT reset
        }
        break;
    case DHT11_STATE_START_SIGNAL:
        // Send START_SIGNAL for at least 18ms, then switch to input mode
        if (current_tick - handle->entry_tick >= DHT11_OFF_START_DURATION)
        {
            /* Reset toan bo state cho chu ky nhan moi truoc khi bat ngat. */
            handle->bit_cnt = 0;
            handle->last_falling = 0;
            handle->raw_data[0] = 0;
            handle->raw_data[1] = 0;
            handle->raw_data[2] = 0;
            handle->raw_data[3] = 0;
            handle->raw_data[4] = 0;
            GPIO_SetPinInput(handle->port, handle->pin);
            handle->state = DHT11_STATE_WAIT_RESPONSE;
            __HAL_TIM_SET_COUNTER(handle->htim, 0);
        }
        else
        {
            GPIO_SetPinOutput(handle->port, handle->pin);
            HAL_GPIO_WritePin(handle->port, handle->pin, GPIO_PIN_RESET);
        }
        break;
    case DHT11_STATE_WAIT_RESPONSE:
        if (current_tick - handle->entry_tick >= handle->timeout_ms)
        {
            handle->state = DHT11_STATE_ERROR;
        }
        break;
    case DHT11_STATE_RECEIVING:
        if (current_tick - handle->entry_tick >= handle->timeout_ms)
        {
            handle->state = DHT11_STATE_ERROR;
        }
        else if (handle->bit_cnt >= 40)
        {
            uint8_t checksum = (uint8_t)(handle->raw_data[0] + handle->raw_data[1] +
                                         handle->raw_data[2] + handle->raw_data[3]);
            handle->state = (checksum == handle->raw_data[4]) ? DHT11_STATE_DONE : DHT11_STATE_ERROR;
        }
        break;
    case DHT11_STATE_DONE:
        // Copy raw_data to humidity and temperature, then go back to IDLE
        handle->humidity = handle->raw_data[0];
        handle->temperature = handle->raw_data[2];
        GPIO_SetPinOutput(handle->port, handle->pin);
        HAL_GPIO_WritePin(handle->port, handle->pin, GPIO_PIN_SET);
        handle->last_task_run = current_tick;
        handle->state = DHT11_STATE_IDLE;
        break;
    case DHT11_STATE_ERROR:
        // Handle error, then go back to IDLE
        GPIO_SetPinOutput(handle->port, handle->pin);
        HAL_GPIO_WritePin(handle->port, handle->pin, GPIO_PIN_SET);
        handle->last_task_run = current_tick;
        handle->state = DHT11_STATE_IDLE;
        break;
    default:
        break;
    }
}
