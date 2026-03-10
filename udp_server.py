#!/usr/bin/env python3
import socket
import sys

HOST = "0.0.0.0"  # listen on all interfaces
PORT = 5005         # must match ESP32 REMOTE_PORT

def main():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((HOST, PORT))
    print(f"UDP server listening on {HOST}:{PORT}")

    while True:
        data, addr = sock.recvfrom(2048)  # blocks until a packet arrives
        # Print raw bytes + best-effort decoded text
        print(f'From {addr[0]}:{addr[1]} | {len(data)} bytes | raw={data.decode("utf-8", errors="replace")}')


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\nCtrl+C pressed. Exiting cleanly.")
        sys.exit(0)   # optional; you can also just "pass" and let it end