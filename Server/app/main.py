from datetime import datetime, timezone
from fastapi import FastAPI, status
from pydantic import BaseModel, Field


app = FastAPI(title="RoomReader API")


class ReadingJSON(BaseModel):
    """Payload that the ESP32 sends to the API."""

    device_id: str = Field(
        min_length=1,
        max_length=80,
        examples=["roomreader-001"],
    )
    recorded_at: datetime | None = Field(
        default=None,
        description="When the ESP32 measured the reading. The server fills this in if omitted.",
    )
    pm1_0: float | None = Field(default=None, ge=0, description="PM1.0 in ug/m3")
    pm2_5: float | None = Field(default=None, ge=0, description="PM2.5 in ug/m3")
    pm10_0: float | None = Field(default=None, ge=0, description="PM10 in ug/m3")
    co2: int | None = Field(default=None, ge=0, description="CO2 in ppm")
    temp_c: float | None = Field(default=None, ge=-40, le=85, description="Temperature in Celsius")
    rh_percent: float | None = Field(
        default=None,
        ge=0,
        le=100,
        description="Relative humidity percentage",
    )
    voc_index: int | None = Field(default=None, ge=0, le=500)
    nox_index: int | None = Field(default=None, ge=0, le=500)
    wifi_rssi: int | None = Field(default=None, ge=-120, le=0, description="WiFi signal in dBm")
    uptime_seconds: int | None = Field(default=None, ge=0)
    firmware_version: str | None = Field(default=None, max_length=40, examples=["0.1.0"])


class StoredReading(ReadingJSON):
    """Reading after the server accepts and stores it."""

    id: int
    received_at: datetime


class CreateReadingResponse(BaseModel):
    status: str
    reading_id: int
    received_at: datetime


# Temporary storage while learning FastAPI.
# Readings are cleared whenever the server restarts because they are stored in RAM.
readings: list[StoredReading] = []


@app.get("/")
def root() -> dict[str, str]:
    return {"message": "RoomReader server is running"}


@app.get("/health")
def health() -> dict[str, str]:
    return {"status": "ok"}


@app.post("/readings", status_code=status.HTTP_201_CREATED, response_model=CreateReadingResponse)
def create_reading(reading: ReadingJSON) -> CreateReadingResponse:
    now = datetime.now(timezone.utc)
    recorded_at = reading.recorded_at or now

    stored_reading = StoredReading(
        id=len(readings) + 1,
        device_id=reading.device_id,
        recorded_at=recorded_at,
        pm1_0=reading.pm1_0,
        pm2_5=reading.pm2_5,
        pm10_0=reading.pm10_0,
        co2=reading.co2,
        temp_c=reading.temp_c,
        rh_percent=reading.rh_percent,
        voc_index=reading.voc_index,
        nox_index=reading.nox_index,
        wifi_rssi=reading.wifi_rssi,
        uptime_seconds=reading.uptime_seconds,
        firmware_version=reading.firmware_version,
        received_at=now,
    )

    readings.append(stored_reading)

    return CreateReadingResponse(
        status="saved",
        reading_id=stored_reading.id,
        received_at=stored_reading.received_at,
    )


@app.get("/readings", response_model=list[StoredReading])
def list_readings() -> list[StoredReading]:
    return readings


@app.get("/readings/latest", response_model=StoredReading | None)
def latest_reading(device_id: str | None = None) -> StoredReading | None:
    matching_readings = [
        reading for reading in readings
        if device_id is None or reading.device_id == device_id
    ]
    if not matching_readings:
        return None

    return matching_readings[-1]
