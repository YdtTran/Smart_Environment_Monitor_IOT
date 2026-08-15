# AGENTS.md

## Project

Smart Environment Monitor & Auto-Vent Control — a 3-node embedded system (STM32 sensor/actuator node → ESP32 gateway node → Raspberry Pi 4 server) that monitors environment conditions and auto-controls a vent servo. Portfolio / learn-by-doing project; correctness and clean architecture matter more than shipping speed.

## Most important rule

Agents only give guidance, review, and scaffolding. Do **not** write implementation logic on your own initiative — only implement when explicitly asked to, in that specific request.

## Coding Guidelines (Karpathy)

Behavioral guidelines to reduce common LLM coding mistakes, derived from [Andrej Karpathy's observations](https://x.com/karpathy/status/2015883857489522876) on LLM coding pitfalls. These bias toward caution over speed — for trivial tasks, use judgment.

### 1. Think Before Coding

Don't assume. Don't hide confusion. Surface tradeoffs.

- State your assumptions explicitly. If uncertain, ask.
- If multiple interpretations exist, present them — don't pick silently.
- If a simpler approach exists, say so. Push back when warranted.
- If something is unclear, stop. Name what's confusing. Ask.

### 2. Simplicity First

Minimum code that solves the problem. Nothing speculative.

- No features beyond what was asked.
- No abstractions for single-use code.
- No "flexibility" or "configurability" that wasn't requested.
- No error handling for impossible scenarios.
- If you write 200 lines and it could be 50, rewrite it.

Ask yourself: "Would a senior engineer say this is overcomplicated?" If yes, simplify.

### 3. Surgical Changes

Touch only what you must. Clean up only your own mess.

When editing existing code:

- Don't "improve" adjacent code, comments, or formatting.
- Don't refactor things that aren't broken.
- Match existing style, even if you'd do it differently.
- If you notice unrelated dead code, mention it — don't delete it.

When your changes create orphans:

- Remove imports/variables/functions that YOUR changes made unused.
- Don't remove pre-existing dead code unless asked.

The test: every changed line should trace directly to the user's request.

### 4. Goal-Driven Execution

Define success criteria. Loop until verified.

Transform tasks into verifiable goals:

- "Add validation" → "Write tests for invalid inputs, then make them pass"
- "Fix the bug" → "Write a test that reproduces it, then make it pass"
- "Refactor X" → "Ensure tests pass before and after"

For multi-step tasks, state a brief plan:

```
1. [Step] → verify: [check]
2. [Step] → verify: [check]
3. [Step] → verify: [check]
```

Strong success criteria let you loop independently. Weak criteria ("make it work") require constant clarification.

## Source of truth for architecture

Read before writing any code — do NOT guess architecture:

- [docs/diagrams/](docs/diagrams/) — C4 context (L1), container (L2), and component (L3, per node) `.drawio` diagrams.
- [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) — Mermaid mirror of those diagrams.
- Notion decision log — source of truth for architecture decisions (not duplicated in this repo).

If a diagram and this file ever disagree, the diagram/Notion log wins.

## Architecture (from docs/ARCHITECTURE.md)

- **STM32F103C8T6 (sensor/actuator node)**: reads DHT11 (one-wire), LDR + 3 potentiometers (ADC — sampling-rate config, LCD scroll-speed config, servo feedback), drives a 360° servo via PWM with bang-bang + hysteresis control, drives a warning LED, writes to an OLED SSD1306 and LCD 16x2 (shared I2C1 bus), and sends processed data to the ESP32 over UART.
- **ESP32 (gateway node, ESP-IDF/FreeRTOS)**: receives UART data from STM32, shows time/uptime/WiFi/MQTT status on an SPI TFT ST7789, and publishes to the RPi4 over MQTT.
- **Raspberry Pi 4 (server, not PlatformIO)**: local Mosquitto broker + Node-RED flows (`mqtt_subscriber`, `thingsboard_forwarder` → ThingsBoard, `local_dashboard` view-only HTTP dashboard).

## Repo structure

```
docs/         diagrams (do not touch), hardware pinouts/wiring, requirements (Notion backup), test-reports, references, logs
firmware/
  stm32/      PlatformIO — platform=ststm32, framework=stm32cube, board=bluepill_f103c8
  esp32/      PlatformIO — platform=espressif32, framework=espidf, board=esp32dev
middleware/
  rpi4/       mosquitto config + Node-RED flows (not PlatformIO)
```

Both firmware projects follow the same per-lib layout: each `lib/<name>/` is one component with its own `README.md` one-liner describing its responsibility; `include/` holds shared headers/config; `test/` holds native host-run unit tests for pure logic (no board required). Each firmware project also has its own `AGENTS.md` for project-specific conventions — see [firmware/stm32/AGENTS.md](firmware/stm32/AGENTS.md) and [firmware/esp32/AGENTS.md](firmware/esp32/AGENTS.md).

## Commands

Run from inside `firmware/stm32/` or `firmware/esp32/` respectively (each is its own PlatformIO project):
Path: `C:\Users\trand\.platformio\penv\Scripts\pio.exe`

```
pio run                       # build
pio run -t upload             # build + flash over ST-Link (stm32) / serial (esp32)
pio run -t clean              # clean build artifacts
pio device monitor             # serial monitor (115200 baud, per platformio.ini)
pio test                      # run native unit tests (test/)
pio test -f <test_name>       # run a single test
```

- STM32 target: `bluepill_f103c8`, framework `stm32cube` (STM32 HAL, not Arduino).
- ESP32 target: `esp32dev`, framework `espidf` — uses FreeRTOS tasks, no Arduino `loop()`.

## Coding conventions (minimum)

- Non-blocking (NFR-2): no blocking `delay()` on STM32.
- ESP32 uses FreeRTOS tasks — no Arduino `loop()`.

## Commit Convention

Dùng Conventional Commits:

```text
<type>(<scope>): <mô tả ngắn, thì hiện tại, không viết hoa đầu, không dấu chấm cuối>
```

- `type`: `feat` (tính năng/component mới), `fix` (sửa bug), `docs` (chỉ đổi tài liệu/README/Notion sync), `chore` (setup, config, dependency), `test` (thêm/sửa test), `refactor` (đổi cấu trúc, không đổi hành vi).
- `scope`: tên node hoặc component — `stm32`, `esp32`, `rpi4`, hoặc cụ thể hơn như `stm32-dht11`, `esp32-mqtt`.
- Ví dụ: `feat(stm32): scaffold dht11_handler lib skeleton`, `fix(esp32-mqtt): retry wifi connect on disconnect`, `docs: log HAL_I2C_Manager rename in decision log`, `chore: add platformio.ini for esp32 project`.
- Mỗi component/lib nên có ít nhất 1 commit riêng khi scaffold xong (dễ review từng phần).
- Commit đầu tiên sau khi scaffold: `chore: initial project structure (PlatformIO + Node-RED + docs)`.

## Git Workflow

1. Trước khi bắt đầu sửa: `git pull --rebase` để lấy thay đổi mới nhất, tránh làm việc trên base cũ.
2. Tạo/chuyển sang branch riêng nếu cần: `<type>/<node>-<mô-tả-ngắn>`, ví dụ `feat/stm32-dht11-handler`, `fix/esp32-mqtt-reconnect`.
3. Edit code/file.
4. `git add` + `git commit` theo đúng Commit Convention ở trên.
5. Trước khi push: `git pull --rebase` lại lần nữa (đề phòng có thay đổi mới trong lúc mình edit) — nếu có conflict thì resolve tại bước này, không resolve lúc push.
6. `git push`.
7. Nhánh `main` không commit thẳng — merge qua branch riêng.
