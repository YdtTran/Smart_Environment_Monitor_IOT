# Smart Environment Monitor & Auto-Vent Control

3-node embedded system (STM32 sensor/actuator node → ESP32 gateway node → Raspberry Pi 4 server) that monitors environment conditions and auto-controls a vent servo. Portfolio / learn-by-doing project.

## Architecture

Architecture and decisions are **not** decided in this README — read before writing any code:

- [docs/diagrams/](docs/diagrams/) — C4 context (L1), container (L2), and component (L3) diagrams per node.
- [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) — Mermaid mirror of the diagrams above.
- Notion decision log — source of truth for architecture decisions (link managed in Notion, not duplicated here).

See [AGENTS.md](AGENTS.md) for how AI agents should work in this repo, coding conventions, and the Git workflow.

## Structure

```
docs/         diagrams, hardware notes, requirements backup, test reports, references, logs
firmware/
  stm32/      PlatformIO project — sensor/actuator node
  esp32/      PlatformIO project — gateway node
middleware/
  rpi4/       Mosquitto + Node-RED middleware (not PlatformIO)
```
