# RoomReader

RoomReader is a research prototype for monitoring environmental conditions
associated with indoor air quality. The system combines particulate matter,
carbon dioxide, temperature, relative humidity, VOC, and NOx measurements in a
single ESP32-C3-based platform.

This repository contains the embedded firmware and the initial backend used to
collect and query measurements. It is an experimental system and is not a
validated medical or diagnostic device.

## System Architecture

```text
Environmental Sensors
        |
        v
ESP32-C3 Firmware
        |
        | JSON over HTTP
        v
FastAPI Server
        |
        v
PostgreSQL Database (planned)
        |
        v
iOS Application (planned)
```

## Sensors

| Sensor | Interface | Measurements |
| --- | --- | --- |
| PMS5003 | UART | PM1.0, PM2.5, and PM10 |
| SCD40 | I2C (`0x62`) | CO2, temperature, and relative humidity |
| SGP41 | I2C (`0x59`) | VOC and NOx indices |

The SCD40 humidity and temperature measurements are supplied to the SGP41 for
on-sensor humidity compensation. The SGP41 Gas Index Algorithm is processed at
1 Hz, while the SCD40 is sampled every 5 s.

## Repository Layout

```text
RoomReader/
|-- ESP32C3/        # PlatformIO firmware and sensor drivers
|-- Server/         # Simple FastAPI JSON template
`-- README.md
```

## Firmware Setup

The firmware requires PlatformIO and an ESP32-C3 development board.

1. Create the local credentials file:

   ```bash
   cp ESP32C3/src/secrets.example.h ESP32C3/src/secrets.h
   ```

2. Replace the placeholder Wi-Fi values in `ESP32C3/src/secrets.h`.
3. Build the firmware:

   ```bash
   cd ESP32C3
   pio run
   ```

4. Upload and open the serial monitor:

   ```bash
   pio run --target upload
   pio device monitor --baud 115200
   ```

`secrets.h` is intentionally excluded from Git.

## Backend Setup

The FastAPI backend is packaged as a Docker Compose service for home servers,
including ARM64 devices such as the Jetson Orin Nano. It automatically restarts,
has an internal health check, and exposes a configurable host port.

```bash
cd Server
cp .env.example .env
docker compose up -d --build
```

The API is available at `http://localhost:8000`, with interactive documentation
at `http://localhost:8000/docs`.

See [`Server/README.md`](Server/README.md) for deployment, configuration,
updates, logs, and manual development instructions.

Current endpoints include:

- `GET /`
- `GET /health`
- `POST /readings`
- `GET /readings`

## Data Collection

The firmware preserves each sensor's required sampling frequency while reducing
transmission frequency. Valid samples are accumulated over a 120-s window,
averaged, and sent to the FastAPI server as one representative record every
2 min. Samples are cleared after a successful upload and retained for the next
attempt if the upload fails.

## Prototype Status

As of August 27, 2026:

- C++ drivers exist for all three sensors.
- The SCD40 is detected at `0x62`.
- The SGP41 is not detected at `0x59`; replacement hardware is pending.
- The PMS5003 currently reports zero-valued particulate measurements and
  requires additional frame, checksum, power, and fan validation.
- Two-minute aggregation and ESP32 HTTP uploads are implemented.
- A simple FastAPI JSON API is implemented with in-memory storage.
- PostgreSQL persistence and authentication remain planned work.
- Hardware validation of end-to-end uploads and the iOS application remain
  planned work.

## Planned Work

1. Validate the replacement SGP41 module.
2. Diagnose the PMS5003 zero-valued measurements.
3. Add the remaining CRC and checksum validation to the PMS5003 and SCD40
   drivers.
4. Validate end-to-end storage and retrieval on hardware.
5. Add automated firmware and API tests.
6. Add PostgreSQL persistence and authentication.
7. Develop the initial iOS visualization interface.
