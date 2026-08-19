# RoomReader Server

This is a deliberately small FastAPI template for learning how the ESP32 can
send JSON to a server.

The complete server is in `app/main.py`. It currently keeps readings in memory
while the request flow is being built. PostgreSQL, authentication, Docker, and
separate route files can be added later after the basic API shape is clear.

## Run the Server

```bash
cd /Users/angellou/RoomReader/Server
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
uvicorn app.main:app --reload
```

## URLs

- API docs: http://localhost:8000/docs
- Health check: http://localhost:8000/health
- Stored readings: http://localhost:8000/readings
- Latest reading: http://localhost:8000/readings/latest

## Edit the Accepted JSON

Open `app/main.py` and find:

```python
class ReadingJSON(BaseModel):
    device_id: str
    recorded_at: datetime | None = None
    pm1_0: float | None = None
    pm2_5: float | None = None
    pm10_0: float | None = None
    co2: int | None = None
    temp_c: float | None = None
    rh_percent: float | None = None
    voc_index: int | None = None
    nox_index: int | None = None
    wifi_rssi: int | None = None
    uptime_seconds: int | None = None
    firmware_version: str | None = None
```

`device_id` is required. Most sensor fields are optional so the ESP32 can still
send a partial reading if one sensor is unavailable. The model also validates
reasonable ranges, such as humidity from 0 to 100 and VOC/NOx index values from
0 to 500.

The server adds:

- `id`: a temporary reading number.
- `received_at`: when the API accepted the reading.
- `recorded_at`: filled in by the server if the ESP32 does not send it.

## Try an Upload

```bash
curl -X POST http://localhost:8000/readings \
  -H "Content-Type: application/json" \
  -d '{
    "device_id": "roomreader-001",
    "pm1_0": 4.0,
    "pm2_5": 7.0,
    "pm10_0": 10.0,
    "co2": 615,
    "temp_c": 24.8,
    "rh_percent": 48.2,
    "voc_index": 92,
    "nox_index": 1,
    "wifi_rssi": -58,
    "uptime_seconds": 4302,
    "firmware_version": "0.1.0"
  }'
```

Read back the latest reading:

```bash
curl "http://localhost:8000/readings/latest?device_id=roomreader-001"
```

The server temporarily stores readings in a Python list. Restarting the server
clears the list.

