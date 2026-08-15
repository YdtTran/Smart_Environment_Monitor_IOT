/**
 * @file    dht11.h
 * @brief   DHT11 driver - kien truc 2 lop (scheduler trigger/poll + interrupt-driven bit read)
 *
 * Layer 1 (Scheduler, tick 1ms, non-blocking):
 *   - DHT11_Task() duoc goi dinh ky boi superloop scheduler.
 *   - Chi lam: trigger (khi den chu ky) + poll state (kiem tra DONE/ERROR) + xu ly timeout.
 *   - KHONG doc bit truc tiep, KHONG blocking delay.
 *
 * Layer 2 (Interrupt, EXTI + Timer Input Capture, do us):
 *   - ISR xu ly toan bo qua trinh nhan 40 bit (5 byte), doc lap voi tick 1ms.
 *   - Chi ghi du lieu (raw_data, bit_cnt, state) - khong goi ham nang/blocking.
 *
 * Giao tiep giua 2 lop: qua cac bien volatile trong DHT11_Handle_t.
 * Task chi doc ket qua khi state == DHT11_STATE_DONE (hoac ERROR).
 */

#ifndef DHT11_H
#define DHT11_H

#include <stdint.h>
#include "stm32f1xx_hal.h" /* GPIO_TypeDef, TIM_HandleTypeDef */

#ifdef __cplusplus
extern "C"
{
#endif

#define DHT11_OFF_START_DURATION 20 /* ms: thoi gian keo line xuong thap de trigger doc */

    /* ============================================================
     *  ENUM
     * ============================================================ */

    /**
     * @brief State machine cho tung chu ky doc DHT11.
     *
     * Transition (xem FSM da chot trong Notion Decision Log):
     *   IDLE          -> START_SIGNAL   (task: trigger, den chu ky sample)
     *   START_SIGNAL  -> WAIT_RESPONSE  (task: da keo LOW du ~18ms, tha GPIO ve input, enable IT)
     *   WAIT_RESPONSE -> RECEIVING      (ISR: phat hien 80us low + 80us high tu sensor)
     *   RECEIVING     -> RECEIVING      (ISR: nhan tung bit, tich luy vao raw_data)
     *   RECEIVING     -> DONE           (ISR: du 40 bit, checksum dung)
     *   RECEIVING     -> ERROR          (ISR: du 40 bit, checksum sai)
     *   {START_SIGNAL, WAIT_RESPONSE, RECEIVING} -> ERROR
     *                                   (task poll: HAL_GetTick() - entry_tick > timeout, dung 1 nguong tong)
     *   DONE          -> IDLE           (task: da copy du lieu ra buffer chinh thuc)
     *   ERROR         -> IDLE           (task: GPIO chuyen OUTPUT de chan edge la, clear pending IT)
     */
    typedef enum
    {
        DHT11_STATE_IDLE = 0,
        DHT11_STATE_START_SIGNAL,  /* MCU dang keo line xuong thap de bat dau */
        DHT11_STATE_WAIT_RESPONSE, /* Cho cam bien phan hoi (80us low + 80us high) */
        DHT11_STATE_RECEIVING,     /* Dang nhan 40 bit qua ngat */
        DHT11_STATE_DONE,          /* Nhan du 40 bit, checksum hop le */
        DHT11_STATE_ERROR          /* Timeout hoac checksum sai */
    } DHT11_State_t;

    /**
     * @brief Ket qua tra ve cho cac ham API public (khac voi DHT11_State_t noi bo).
     */
    typedef enum
    {
        DHT11_OK = 0,
        DHT11_ERR_NOT_READY, /* Chua co du lieu moi (state != DONE) */
        DHT11_ERR_TIMEOUT,   /* Lan doc gan nhat bi timeout */
        DHT11_ERR_CHECKSUM,  /* Lan doc gan nhat checksum sai */
        DHT11_ERR_BUSY       /* Dang trong qua trinh doc (khong the trigger lai) */
    } DHT11_Status_t;

    /* ============================================================
     *  STRUCT
     * ============================================================ */

    /**
     * @brief Handle chua toan bo state noi bo cua 1 instance DHT11.
     *
     * Cac field volatile duoc chia se giua ISR (Layer 2) va Task (Layer 1) -
     * truy cap can dat trong doan khong bi ngat (hoac chap nhan rui ro da danh gia)
     * tuy theo chien luoc dong bo ban chon khi implement.
     */
    typedef struct
    {
        /* --- Cau hinh phan cung (set 1 lan luc init, khong doi khi chay) --- */
        GPIO_TypeDef *port;        /* GPIO port cua chan data */
        uint16_t pin;              /* GPIO pin cua chan data */
        TIM_HandleTypeDef *htim;   /* Timer do us, free-running, dung cho Input Capture */
        uint32_t timer_channel;    /* VD: TIM_CHANNEL_1 - kenh Input Capture dang dung */
        uint32_t sample_period_ms; /* Chu ky trigger (VD: 2000ms theo datasheet) */
        uint32_t timeout_ms;       /* Nguong timeout tong cho 1 chu ky doc */

        /* --- State runtime (chia se ISR <-> Task) --- */
        volatile DHT11_State_t state;   /* State hien tai cua FSM */
        volatile uint8_t bit_cnt;       /* So bit da nhan (0-40), ISR cap nhat */
        volatile uint32_t last_falling; /* us: thoi diem canh rising truoc do (tu htim->CNT) */
        volatile uint8_t raw_data[5];   /* Buffer tho: hum_int, hum_dec, temp_int, temp_dec, checksum */
        volatile uint32_t entry_tick;   /* ms: HAL_GetTick() luc bat dau chu ky (vao START_SIGNAL) */

        /* --- Ket qua da xac nhan (chi Task ghi, sau khi copy tu raw_data) --- */
        uint8_t humidity;       /* % RH, gia tri hop le gan nhat */
        uint8_t temperature;    /* degC, gia tri hop le gan nhat */
        uint32_t last_task_run; /* ms: lastRun cho superloop (elapsed >= period check) */
    } DHT11_Handle_t;

    /* ============================================================
     *  API - goi tu Layer 1 (Scheduler / Task, main loop)
     * ============================================================ */

    /**
     * @brief Khoi tao handle + cau hinh GPIO/timer ban dau (chua trigger doc).
     * @param handle    Con tro handle can init.
     * @param port      GPIO port cua chan data.
     * @param pin       GPIO pin cua chan data.
     * @param htim      Timer dung cho Input Capture (da duoc MX_TIMx_Init() truoc do).
     * @param channel   Kenh Input Capture (VD: TIM_CHANNEL_1).
     * @param sample_period_ms  Chu ky lay mau (goi y datasheet: >= 1000ms, thuc te dung 2000ms).
     * @param timeout_ms         Nguong timeout tong cho 1 chu ky doc.
     * @return DHT11_OK neu init thanh cong.
     */
    DHT11_Status_t DHT11_Init(DHT11_Handle_t *handle,
                              GPIO_TypeDef *port, uint16_t pin,
                              TIM_HandleTypeDef *htim, uint32_t channel,
                              uint32_t sample_period_ms, uint32_t timeout_ms);

    /**
     * @brief Task chinh, goi dinh ky trong superloop (VD: moi 1ms hoac moi vong lap).
     *        Tu kiem tra elapsed >= sample_period_ms de quyet dinh trigger.
     *        Tu kiem tra state == DONE/ERROR de xu ly ket qua.
     *        Tu kiem tra timeout dua tren entry_tick.
     *        Non-blocking - luon return nhanh.
     * @param handle Con tro handle.
     */
    void DHT11_Read(DHT11_Handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif /* DHT11_H */