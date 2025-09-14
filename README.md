# Nano Satellite

Nano Satellite Simulator — Embedded STM32 Firmware + C++ Ground Station

---

## Overview

- **STM32 firmware** collects sensor data, handles modes (Normal/Error/Safe), and communicates over UART.
- **C++ Gateway** relays UART communication to multiple TCP clients.
- **Client CLI** sends commands and receives telemetry.

---

## Main Features

- Beaconing and telemetry transmission
- Sensor sampling: Temperature, Humidity, Light
- SD card logging via FatFS
- Operational mode control (Normal/Error/Safe)
- UART packet-based protocol
- TCP server for client connections

---

## UART Packet Format

| Field      | Size  | Description         |
|------------|-------|---------------------|
| Length     | 1 B   | Payload + ID + CRC   |
| Packet ID  | 1 B   | Command/Response ID  |
| Payload    | 0-254B| Payload data         |
| Checksum   | 1 B   | XOR checksum         |
| End Byte   | 1 B   | 0x55 constant        |

---

## Main Tasks

| Task                  | Purpose                                               |
|-----------------------|-------------------------------------------------------|
| `init_task`           | Sync time, initialize components, launch tasks.       |
| `collector_task`      | Sample sensors: temperature, humidity, light.         |
| `keep_alive_task`     | Send periodic status to ground server                 |
| `logger_task`         | Store sampled data on SD card.                        |
| `event_task`          | Log system events (init, status changes) to file.     |
| `flash_task`          | Persist modifiable settings to flash.                 |
| `uart_task`           | Manage UART packet reception and transmission         |


---

## Build

### NanoSat Firmware
- STM32CubeIDE Project
- FreeRTOS based
- Flash binary to STM32 board
- UART configured at 115200 baud rate

### Gateway Server + Client
- C++17
- Build with `g++`