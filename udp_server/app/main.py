#!/usr/bin/env python3
import os
import socket
import sqlite3
import threading
import time
from datetime import datetime, timezone
from typing import List, Dict, Any

from fastapi import FastAPI
from fastapi.responses import HTMLResponse, JSONResponse
from fastapi.templating import Jinja2Templates
from fastapi.requests import Request

UDP_HOST = os.getenv("UDP_HOST", "0.0.0.0")
UDP_PORT = int(os.getenv("UDP_PORT", "5005"))
DB_PATH = os.getenv("DB_PATH", "/data/messages.db")

app = FastAPI(title="UDP Logger")
templates = Jinja2Templates(directory="app/templates")

stop_event = threading.Event()


def db_connect() -> sqlite3.Connection:
    # check_same_thread=False permite usar la conexión desde distintos hilos
    conn = sqlite3.connect(DB_PATH, check_same_thread=False)
    conn.row_factory = sqlite3.Row
    return conn


def db_init(conn: sqlite3.Connection) -> None:
    conn.execute(
        """
        CREATE TABLE IF NOT EXISTS messages (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            ts_utc TEXT NOT NULL,
            src_ip TEXT NOT NULL,
            src_port INTEGER NOT NULL,
            bytes_len INTEGER NOT NULL,
            text TEXT NOT NULL
        )
        """
    )
    conn.execute("CREATE INDEX IF NOT EXISTS idx_messages_ts ON messages(ts_utc)")
    conn.commit()


def insert_message(conn: sqlite3.Connection, src_ip: str, src_port: int, data: bytes) -> None:
    text = data.decode("utf-8", errors="replace")
    ts_utc = datetime.now(timezone.utc).isoformat()
    conn.execute(
        "INSERT INTO messages (ts_utc, src_ip, src_port, bytes_len, text) VALUES (?, ?, ?, ?, ?)",
        (ts_utc, src_ip, src_port, len(data), text),
    )
    conn.commit()


def udp_listener() -> None:
    conn = db_connect()
    db_init(conn)

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((UDP_HOST, UDP_PORT))
    sock.settimeout(1.0)  # para poder salir limpio cuando pare el container

    print(f"[UDP] Listening on {UDP_HOST}:{UDP_PORT} (UDP)")
    try:
        while not stop_event.is_set():
            try:
                data, addr = sock.recvfrom(2048)
            except socket.timeout:
                continue

            src_ip, src_port = addr[0], addr[1]
            # Log en consola + insert
            print(f"[UDP] From {src_ip}:{src_port} | {len(data)} bytes | {data.decode('utf-8', errors='replace')}")
            insert_message(conn, src_ip, src_port, data)
    finally:
        try:
            sock.close()
        except Exception:
            pass
        try:
            conn.close()
        except Exception:
            pass
        print("[UDP] Listener stopped.")


@app.on_event("startup")
def on_startup():
    # Asegura DB + tabla antes de arrancar
    conn = db_connect()
    db_init(conn)
    conn.close()

    # Arranca UDP listener en background
    t = threading.Thread(target=udp_listener, daemon=True)
    t.start()


@app.on_event("shutdown")
def on_shutdown():
    stop_event.set()
    time.sleep(1.1)  # deja que el hilo salga por el timeout


@app.get("/", response_class=HTMLResponse)
def home(request: Request):
    return templates.TemplateResponse("index.html", {"request": request, "udp_host": UDP_HOST, "udp_port": UDP_PORT})


@app.get("/api/messages")
def get_messages(limit: int = 200) -> List[Dict[str, Any]]:
    limit = max(1, min(limit, 2000))
    conn = db_connect()
    rows = conn.execute(
        "SELECT id, ts_utc, src_ip, src_port, bytes_len, text FROM messages ORDER BY id DESC LIMIT ?",
        (limit,),
    ).fetchall()
    conn.close()
    return [dict(r) for r in rows]


@app.delete("/api/messages")
def delete_messages():
    conn = db_connect()
    conn.execute("DELETE FROM messages")
    conn.commit()
    conn.close()
    return JSONResponse({"ok": True})
