# RoomReader Server

This is a deliberately small FastAPI template for learning how the ESP32 can
send JSON to a server.

The complete server is in `app/main.py`. PostgreSQL, authentication, Docker, and
separate route files can be added later after the basic request flow is clear.

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

## Edit the Accepted JSON

Open `app/main.py` and find:

```python
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
```

Every field currently has a type and no default value, so every field is
required in each uploaded JSON object. A field can be made optional later by
using a declaration such as `voc_index: int | None = None`.

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
    "nox_index": 1
  }'
```

The server temporarily stores readings in a Python list. Restarting the server
clears the list.


