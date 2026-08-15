# AGENTS.md — firmware/stm32

Project-specific notes for the STM32 sensor/actuator node. General rules (most important rule, commit convention, git workflow) live in the root [AGENTS.md](../../AGENTS.md) — this file only covers what's specific to this PlatformIO project.

## Environment

`[env:bluepill_f103c8]` — `platform=ststm32`, `board=bluepill_f103c8`, `framework=stm32cube` (STM32 HAL, not Arduino), `monitor_speed=115200`.

## Config headers (`include/`)

`gpio_config.h`, `i2c_config.h`, `adc_config.h`, `pwm_config.h`, `uart_config.h`, `main.h` — shared pin/peripheral config. Read the matching header before touching a peripheral; don't hardcode pins/handles that already have a config entry.

## Libs (`lib/`)

- `dht11_handler` — reads temperature/humidity from the DHT11 sensor over the one-wire protocol.
- `adc_read` — reads analog channels: sampling-rate potentiometer, LCD scroll-speed potentiometer, servo feedback potentiometer, and the LDR light sensor.
- `hal_i2c_manager` — owns the shared I2C1 bus used by both the OLED (SSD1306) and the LCD 16x2 displays. Don't open a second/competing I2C1 handle elsewhere — go through this lib.
- `data_process` — aggregates and processes raw sensor/ADC readings into the data struct consumed by the rest of the node.
- `decision_block` — applies bang-bang control with hysteresis to drive the servo, and threshold logic to drive the warning LED.
- `uart_frame` — builds the UART frame format used to send processed data to the ESP32 gateway node.
- `scheduler` — non-blocking task loop that schedules the node's periodic work without blocking delays (NFR-2).

## Deploy script (`scripts/`)

`deploy_via_rpi.sh` (bash) / `deploy_via_rpi.bat` (Windows) build locally with PlatformIO, then ship `firmware.bin` to a Raspberry Pi over SSH/SCP and flash it over an ST-Link probe attached to the Pi (`st-flash`, requires `stlink-tools` on the Pi). This is the single build+deploy entrypoint for that flow — use it instead of running `pio run`/`upload` as separate steps. Config comes from `scripts/.env` (copy from `scripts/.env.example`, gitignored) or exported env vars: `RPI_HOST`, `RPI_USER`, `RPI_PORT`, `RPI_REMOTE_DIR`, `PIO_ENV`, `FLASH_ADDR`, and `PIO_BIN` (`.bat` only — defaults to the `pio.exe` path from the root AGENTS.md Commands section, since it isn't on `PATH` by default on Windows).
