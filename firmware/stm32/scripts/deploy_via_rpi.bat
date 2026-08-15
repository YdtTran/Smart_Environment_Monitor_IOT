@echo off
REM Build the STM32 firmware locally with PlatformIO, ship the binary to the
REM Raspberry Pi 4 over SSH/SCP (Windows OpenSSH client), then flash it to the
REM STM32F103C8T6 over an ST-Link probe attached to the Pi's USB, using
REM st-flash (stlink-tools).
REM
REM Requires stlink-tools installed on the Pi: sudo apt install stlink-tools
REM
REM Config: copy scripts\.env.example to scripts\.env and fill in real values
REM (scripts\.env is gitignored), or set the same variables before running.
setlocal enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
for %%I in ("%SCRIPT_DIR%..") do set "STM32_DIR=%%~fI"

if exist "%SCRIPT_DIR%.env" (
    for /f "usebackq eol=# tokens=1,* delims==" %%A in ("%SCRIPT_DIR%.env") do (
        set "%%A=%%B"
    )
)

if not defined RPI_HOST (
    echo error: set RPI_HOST ^(hostname or IP of the Raspberry Pi^) 1>&2
    exit /b 1
)
if not defined RPI_USER set "RPI_USER=pi"
if not defined RPI_PORT set "RPI_PORT=22"
if not defined RPI_REMOTE_DIR set "RPI_REMOTE_DIR=/tmp/stm32-firmware"
if not defined PIO_ENV set "PIO_ENV=bluepill_f103c8"
if not defined FLASH_ADDR set "FLASH_ADDR=0x08000000"
if not defined PIO_BIN set "PIO_BIN=C:\Users\trand\.platformio\penv\Scripts\pio.exe"

set "BIN_FILE=%STM32_DIR%\.pio\build\%PIO_ENV%\firmware.bin"

echo ==^> Building (%PIO_ENV%)
"%PIO_BIN%" run -d "%STM32_DIR%" -e "%PIO_ENV%"
if errorlevel 1 exit /b 1

if not exist "%BIN_FILE%" (
    echo error: %BIN_FILE% not found after build 1>&2
    exit /b 1
)

echo ==^> Copying firmware.bin to %RPI_USER%@%RPI_HOST%:%RPI_REMOTE_DIR%
ssh -p %RPI_PORT% %RPI_USER%@%RPI_HOST% "mkdir -p '%RPI_REMOTE_DIR%'"
if errorlevel 1 exit /b 1

scp -P %RPI_PORT% "%BIN_FILE%" %RPI_USER%@%RPI_HOST%:%RPI_REMOTE_DIR%/firmware.bin
if errorlevel 1 exit /b 1

echo ==^> Flashing over ST-Link on %RPI_HOST% (st-flash)
ssh -p %RPI_PORT% %RPI_USER%@%RPI_HOST% "st-flash --reset write '%RPI_REMOTE_DIR%/firmware.bin' %FLASH_ADDR%"
if errorlevel 1 exit /b 1

echo ==^> Done
endlocal
