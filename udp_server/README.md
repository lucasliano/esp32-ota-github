# UDP Logger (Python + SQLite + Front-end) — Docker

Este proyecto recibe mensajes por **UDP**, los registra en una base **SQLite** (persistente en un volumen Docker) y expone un **front-end web** para visualizar los mensajes y borrar los registros.

## Requisitos

- Docker
- Docker Compose (plugin `docker compose`)

## Estructura

```
udp-logger/
├─ docker-compose.yml
├─ Dockerfile
├─ requirements.txt
└─ app/
   ├─ main.py
   └─ templates/
      └─ index.html
```

## Levantar el servicio

Desde la carpeta del proyecto:

```bash
docker compose up --build
```

Luego abre:

- UI web: http://localhost:8000
- UDP listener: `localhost:5005/udp`

> Si corres esto en otra máquina (por ejemplo un servidor), reemplaza `localhost` por la IP/hostname de ese host.

## Probar envío UDP

### Linux / macOS

```bash
echo "hola desde UDP" | nc -u -w1 127.0.0.1 5005
```

### Windows (PowerShell)

Una opción es usar Python localmente:

```powershell
python - << 'PY'
import socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.sendto(b"hola desde windows", ("127.0.0.1", 5005))
print("enviado")
PY
```

## Borrar registros

En la UI web presiona **“Borrar registros”** para ejecutar `DELETE FROM messages`.

## Persistencia de la base de datos

La base queda en un volumen llamado `udp_logger_data`. Aunque apagues el contenedor, los mensajes quedan guardados.

Para borrar la data persistente (opcional):

```bash
docker compose down -v
```

## Variables de entorno (opcional)

En `docker-compose.yml` puedes ajustar:

- `UDP_HOST` (por defecto `0.0.0.0`)
- `UDP_PORT` (por defecto `5005`)
- `DB_PATH` (por defecto `/data/messages.db`)
