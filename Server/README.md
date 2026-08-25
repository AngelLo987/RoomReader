# RoomReader Server

The RoomReader API accepts environmental readings from an ESP32 and makes them
available over HTTP. Docker Compose is the recommended deployment method for a
Jetson, mini PC, NAS, or other always-on home server. The official Python base
image supports both 64-bit Intel/AMD and ARM systems.

> Readings are currently held in memory. Restarting or upgrading the container
> clears them. Persistent database storage is planned.

## Home-server deployment

Install Git and Docker with the Compose plugin, then clone the repository:

```bash
git clone https://github.com/AngelLo987/RoomReader.git
cd RoomReader/Server
cp .env.example .env
docker compose up -d --build
```

Verify the API:

```bash
curl http://localhost:8000/health
```

Expected response:

```json
{"status":"ok"}
```

Point the ESP32 `baseURL` setting at the home server's LAN address, for example:

```cpp
constexpr char baseURL[] = "http://10.0.0.171:8000";
```

The Compose service starts automatically after Docker restarts and is restarted
if it exits unexpectedly. It also includes an internal health check and runs as
an unprivileged user.

## Configuration

To use a different host port, edit `.env` before starting the service:

```dotenv
ROOMREADER_PORT=8080
```

The ESP32 URL would then end in `:8080`.

## Operations

Check service health and status:

```bash
docker compose ps
curl http://localhost:8000/health
```

Follow server logs:

```bash
docker compose logs -f roomreader-api
```

Restart or stop the service:

```bash
docker compose restart
docker compose down
```

Update after new code is pushed to GitHub:

```bash
git pull
docker compose up -d --build
```

## API URLs

- Documentation: `http://SERVER_IP:8000/docs`
- Health: `http://SERVER_IP:8000/health`
- All readings: `http://SERVER_IP:8000/readings`
- Latest reading: `http://SERVER_IP:8000/readings/latest`

Get the latest reading from PowerShell:

```powershell
Invoke-RestMethod http://SERVER_IP:8000/readings/latest | Format-List
```

## Manual development setup

Docker is not required for local development:

```bash
python3 -m venv .venv
.venv/bin/python -m pip install -r requirements.txt
.venv/bin/python -m uvicorn app.main:app --reload
```
