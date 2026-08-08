# AGENTS.md

## Project

Smart Environment Monitor & Auto-Vent Control — a 3-node embedded system (STM32 sensor/actuator node, ESP32 gateway node, Raspberry Pi 4 server). Portfolio / learn-by-doing project; correctness and clean architecture matter more than shipping speed.

## Source of truth for architecture

Read before writing any code — do NOT guess architecture:

- [docs/diagrams/](docs/diagrams/) — C4 context (L1), container (L2), and component (L3) diagrams.
- Notion decision log — the source of truth for architecture decisions.

If a diagram and this file ever disagree, the diagram/Notion log wins.

## Most important rule

Agents only give guidance, review, and scaffolding. Do **not** write implementation logic on your own initiative — only implement when explicitly asked to, in that specific request.

## Directory structure (summary)

```
docs/         diagrams (do not touch), hardware, requirements (Notion backup), test-reports, references, logs
firmware/
  stm32/      PlatformIO — platform=ststm32, framework=stm32cube, board=bluepill_f103c8
  esp32/      PlatformIO — platform=espressif32, framework=espidf, board=esp32dev
middleware/
  rpi4/       mosquitto config + Node-RED flows (not PlatformIO)
```

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
