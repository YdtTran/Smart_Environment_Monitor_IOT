#!/usr/bin/env bash
# Build the STM32 firmware locally with PlatformIO, ship the binary to the
# Raspberry Pi 4 over SSH/SCP, then flash it to the STM32F103C8T6 over an
# ST-Link probe attached to the Pi's USB, using st-flash (stlink-tools).
#
# Requires stlink-tools installed on the Pi: sudo apt install stlink-tools
#
# Config: copy scripts/.env.example to scripts/.env and fill in real values
# (scripts/.env is gitignored), or export the same variables before running.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
STM32_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

if [ -f "$SCRIPT_DIR/.env" ]; then
  # shellcheck disable=SC1091
  source "$SCRIPT_DIR/.env"
fi

RPI_HOST="${RPI_HOST:?set RPI_HOST (hostname or IP of the Raspberry Pi)}"
RPI_USER="${RPI_USER:-pi}"
RPI_PORT="${RPI_PORT:-22}"
RPI_REMOTE_DIR="${RPI_REMOTE_DIR:-/tmp/stm32-firmware}"
PIO_ENV="${PIO_ENV:-bluepill_f103c8}"
FLASH_ADDR="${FLASH_ADDR:-0x08000000}"

BIN_FILE="$STM32_DIR/.pio/build/$PIO_ENV/firmware.bin"

echo "==> Building ($PIO_ENV)"
pio run -d "$STM32_DIR" -e "$PIO_ENV"

if [ ! -f "$BIN_FILE" ]; then
  echo "error: $BIN_FILE not found after build" >&2
  exit 1
fi

echo "==> Copying firmware.bin to $RPI_USER@$RPI_HOST:$RPI_REMOTE_DIR"
ssh -p "$RPI_PORT" "$RPI_USER@$RPI_HOST" "mkdir -p '$RPI_REMOTE_DIR'"
scp -P "$RPI_PORT" "$BIN_FILE" "$RPI_USER@$RPI_HOST:$RPI_REMOTE_DIR/firmware.bin"

echo "==> Flashing over ST-Link on $RPI_HOST (st-flash)"
ssh -p "$RPI_PORT" "$RPI_USER@$RPI_HOST" \
  "st-flash --reset write '$RPI_REMOTE_DIR/firmware.bin' $FLASH_ADDR"

echo "==> Done"
