# rpi4 middleware

Linux-side middleware running on the Raspberry Pi 4 server: local Mosquitto MQTT broker and Node-RED flows. Not a PlatformIO project.

- `mosquitto/` — local broker config.
- `node-red/flows/` — `mqtt_subscriber`, `thingsboard_forwarder` (MQTT → ThingsBoard), `local_dashboard` (view-only HTTP dashboard).
- `node-red/settings/` — Node-RED settings.
