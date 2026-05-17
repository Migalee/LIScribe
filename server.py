# LIScribe – Server WebSocket v3
# pip install pyserial websockets
# Avvio: python server.py  →  poi apri index.html nel browser

import asyncio
import websockets
import serial
import json
import time
import threading

PORTA = "COM3"
BAUD  = 115200

# Dati sensori: aggiornati dal thread seriale
latest_data    = {"indice":0,"medio":0,"angX":0.0,"angY":0.0,"angZ":0.0}
clients        = set()
ser            = None
pending_cal    = None   # messaggio di calibrazione da spedire al browser


# Tag che vengono intercettati dalla seriale e inoltrati al browser
CAL_TAGS = {"FLEX_APRI","FLEX_CHIUDI","FLEX_OK","GYRO_CAL","CALIBRATO","START","ERRORE_MPU"}


def serial_thread():
    global latest_data, ser, pending_cal
    while True:
        try:
            ser = serial.Serial(PORTA, BAUD, timeout=1)
            time.sleep(2)
            ser.flushInput()
            print(f"[OK] Arduino connesso su {PORTA}")
            while True:
                try:
                    raw  = ser.readline()
                    line = raw.decode("utf-8", errors="ignore").strip()
                    if not line:
                        continue

                    # Controlla se è un messaggio di calibrazione
                    tag = line.split(":")[0].strip()
                    if tag in CAL_TAGS:
                        pending_cal = tag
                        print(f"[CAL] {tag}")
                        continue

                    # Scarta righe non CSV
                    if "," not in line:
                        continue

                    parts = line.split(",")
                    if len(parts) == 5:
                        latest_data = {
                            "indice": int(float(parts[0])),
                            "medio":  int(float(parts[1])),
                            "angX":   float(parts[2]),
                            "angY":   float(parts[3]),
                            "angZ":   float(parts[4]),
                        }
                except Exception:
                    pass

        except serial.SerialException as e:
            print(f"[ERR] {e} — riprovo tra 3s...")
            time.sleep(3)


async def handler(websocket):
    clients.add(websocket)
    print(f"[+] Browser connesso. Totale: {len(clients)}")
    try:
        async for msg in websocket:
            if not ser or not ser.is_open:
                continue
            if msg == "R":
                ser.write(b"R")
                print("[>] Ricalibra giroscopio.")
            elif msg == "F":
                ser.write(b"F")
                print("[>] Ricalibra flex sensor.")
    except websockets.exceptions.ConnectionClosed:
        pass
    finally:
        clients.discard(websocket)
        print(f"[-] Browser disconnesso. Totale: {len(clients)}")


async def broadcaster():
    global pending_cal
    while True:
        if clients:
            payload = dict(latest_data)
            if pending_cal:
                payload["cal"] = pending_cal
                pending_cal = None
            try:
                msg = json.dumps(payload)
                await asyncio.gather(
                    *[c.send(msg) for c in clients.copy()],
                    return_exceptions=True,
                )
            except Exception:
                pass
        await asyncio.sleep(0.05)   # 20 Hz broadcast


async def main():
    threading.Thread(target=serial_thread, daemon=True).start()
    async with websockets.serve(handler, "localhost", 8765):
        print("=" * 46)
        print("  LIScribe Server — ws://localhost:8765")
        print("  Apri index.html nel browser")
        print("=" * 46)
        await broadcaster()


if __name__ == "__main__":
    asyncio.run(main())
