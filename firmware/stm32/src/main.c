#include "main.h"
#include <stdio.h>

TIM_HandleTypeDef htim2;

ssd1306_t oled_display = {
    .buffer = {0},
    .width = SSD1306_WIDTH,
    .height = SSD1306_HEIGHT};

DHT11_Handle_t dht11_handle;

#define DHT11_BIT_THRESHOLD_US 100 /* us: nguong phan biet bit0 (~76-78us) / bit1 (~120us) tren khoang falling-to-falling */

static void SystemClock_Config(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    GPIO_Config_Init();
    MX_I2C1_Init();
    MX_ADC1_Init();
    MX_ServoPWM_Init();
    MX_UART_ESP32_Init();

    SSD1306_Init(&hi2c1);
    SSD1306_Clear(&oled_display);
    SSD1306_UpdateScreen(&hi2c1, &oled_display);

    htim2.Instance = TIM2;
    DHT11_Init(&dht11_handle, DHT11_GPIO_Port, DHT11_Pin, &htim2, TIM_CHANNEL_1, 2000, 1000);

    uint32_t last_display_tick = 0;
    char line[24];
    while (1)
    {
        /* TODO: scheduler_run() once lib/scheduler is implemented. */
        DHT11_Read(&dht11_handle);

        uint32_t now = HAL_GetTick();
        if (now - last_display_tick >= 500)
        {
            last_display_tick = now;
            SSD1306_Clear(&oled_display);
            snprintf(line, sizeof(line), "Humidity: %u %%", dht11_handle.humidity);
            SSD1306_WriteString(&oled_display, 0, 0, line, SSD1306_COLOR_WHITE);
            snprintf(line, sizeof(line), "Temp: %u C", dht11_handle.temperature);
            SSD1306_WriteString(&oled_display, 0, 16, line, SSD1306_COLOR_WHITE);
            SSD1306_UpdateScreen(&hi2c1, &oled_display);
        }

        HAL_GPIO_WritePin(WARNING_LED_GPIO_Port, WARNING_LED_Pin, GPIO_PIN_RESET);
    }
}

static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}

void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
        HAL_GPIO_TogglePin(WARNING_LED_GPIO_Port, WARNING_LED_Pin);
        HAL_Delay(500);
    }
}

void DMA1_Channel6_IRQHandler(void) { HAL_DMA_IRQHandler(&hdma_i2c1_tx); }
void I2C1_EV_IRQHandler(void) { HAL_I2C_EV_IRQHandler(&hi2c1); }
void I2C1_ER_IRQHandler(void) { HAL_I2C_ER_IRQHandler(&hi2c1); }
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    // set flag hoặc release semaphore, báo hiệu SendData đã hoàn tất
    // HAL_GPIO_TogglePin(WARNING_LED_GPIO_Port, WARNING_LED_Pin); // ví dụ: toggle LED
}

void HAL_GPIO_EXTI_Callback(uint16_t gpio_pin)
{
    if (gpio_pin != DHT11_Pin)
    {
        return;
    }

    uint32_t now = __HAL_TIM_GET_COUNTER(dht11_handle.htim);

    if (dht11_handle.state == DHT11_STATE_WAIT_RESPONSE)
    {
        if (dht11_handle.last_falling == 0)
        {
            /* Canh falling #1: bat dau 80us LOW cua tin hieu phan hoi tu cam bien. */
            dht11_handle.last_falling = now;
        }
        else
        {
            /* Canh falling #2: ket thuc 80us HIGH phan hoi, bat dau LOW cua bit 0.
             * Day moi la moc chuan de bat dau do khoang cach giua cac bit. */
            dht11_handle.last_falling = now;
            dht11_handle.state = DHT11_STATE_RECEIVING;
        }
        return;
    }

    if (dht11_handle.state == DHT11_STATE_RECEIVING && dht11_handle.bit_cnt < 40)
    {
        uint32_t pulse_width = (now >= dht11_handle.last_falling)
                                    ? (now - dht11_handle.last_falling)
                                    : ((0xFFFFu - dht11_handle.last_falling) + now + 1u);
        dht11_handle.last_falling = now;

        uint8_t bit = (pulse_width > DHT11_BIT_THRESHOLD_US) ? 1u : 0u;
        dht11_handle.raw_data[dht11_handle.bit_cnt / 8] <<= 1;
        dht11_handle.raw_data[dht11_handle.bit_cnt / 8] |= bit;
        dht11_handle.bit_cnt++;
    }
}