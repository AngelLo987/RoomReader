"""Simple FastAPI template for receiving RoomReader JSON data."""

from fastapi import FastAPI, status
from pydantic import BaseModel


app = FastAPI(title="RoomReader API")

# Payload that the ESP32 will send.
class ReadingJSON(BaseModel):

    device_id: str
    pm1_0: float
    pm2_5: float
    pm10_0: float
    co2: int
    temp_c: float
    rh_percent: float
    voc_index: int
    nox_index: int


# Temporary storage while learning FastAPI.
# Readings are cleared whenever the server restarts because they are stored in RAM.
readings: list[ReadingJSON] = []


@app.get("/")
def root() -> dict[str, str]:
    return {"message": "RoomReader server is running"}


@app.get("/health")
def health() -> dict[str, str]:
    return {"status": "ok"}


@app.post("/readings", status_code=status.HTTP_201_CREATED)
def create_reading(reading: ReadingJSON) -> dict[str, int | str]:
    readings.append(reading)
    return {
        "status": "saved",
        "reading_number": len(readings),
    }


@app.get("/readings")
def list_readings() -> list[ReadingJSON]:
    return readings
