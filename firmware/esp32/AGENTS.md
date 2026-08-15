# AGENTS.md — firmware/esp32

Project-specific notes for the ESP32 gateway node. General rules (most important rule, commit convention, git workflow) live in the root [AGENTS.md](../../AGENTS.md) — this file only covers what's specific to this PlatformIO project.

## Environment

`[env:esp32dev]` — `platform=espressif32`, `board=esp32dev`, `framework=espidf`, `monitor_speed=115200`. ESP-IDF/FreeRTOS — model new work as tasks, not an Arduino `loop()`.

## Libs (`lib/`)

- `uart_receiver_task` — FreeRTOS task that receives UART frames sent from the STM32 sensor node.
- `shared_state` — mutex-protected latest data snapshot shared between the ESP32 tasks. Cross-task data must go through this lib, not ad-hoc globals.
- `wifi_manager_task` — FreeRTOS task that manages the WiFi connection lifecycle for the gateway node.
- `mqtt_publisher_task` — FreeRTOS task that publishes the shared data snapshot to the Raspberry Pi 4 broker over MQTT.
- `tft_display_task` — FreeRTOS task that drives the SPI TFT ST7789, showing time, uptime, WiFi status, and MQTT protocol status.
