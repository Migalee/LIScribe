# LIScribe – Lettura dati seriali
# Python 3.14 – pyserial

import serial
import time

PORTA = "COM3"
BAUD  = 115200

def leggi_dati(ser):
    try:
        riga = ser.readline().decode("utf-8").strip()
        if not riga or "," not in riga:
            return None
        valori = riga.split(",")
        if len(valori) != 6:
            return None
        return {
            "A2": int(valori[0]),
            "A1": int(valori[1]),
            "A0": int(valori[2]),
            "GX": float(valori[3]),
            "GY": float(valori[4]),
            "GZ": float(valori[5]),
        }
    except (ValueError, UnicodeDecodeError):
        return None

def main():
    print("LIScribe avviato.")
    input("Premi INVIO per iniziare la lettura...")

    try:
        ser = serial.Serial(PORTA, BAUD, timeout=1)
        time.sleep(2)
        ser.flushInput()
        print("Connesso. Premi CTRL+C per fermare.\n")
    except serial.SerialException as e:
        print(f"Errore connessione: {e}")
        return

    try:
        while True:
            dati = leggi_dati(ser)
            if dati:
                print(
    f"A2={dati['A2']:3d}%  "
    f"A1={dati['A1']:3d}%  "
    f"A0={dati['A0']:3d}%  |  "
    f"GX={dati['GX']:7.3f}  "
    f"GY={dati['GY']:7.3f}  "
    f"GZ={dati['GZ']:7.3f}"
)
    except KeyboardInterrupt:
        print("\nLettura fermata.")
    finally:
        ser.close()
        print("Porta seriale chiusa.")

if __name__ == "__main__":
    main()