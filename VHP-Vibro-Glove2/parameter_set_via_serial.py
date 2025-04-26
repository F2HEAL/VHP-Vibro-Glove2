import serial
import time
import argparse

# === Configuration ===
DEFAULT_SERIAL_PORT = 'COM17'    # Change default if needed
BAUDRATE = 115200
TIMEOUT_SEC = 1

def send_command(ser, command):
    ser.write((command + '\n').encode('utf-8'))
    print(f"Sent: {command}")

    # Wait a little to receive Arduino reply
    time.sleep(0.05)

    # Read and print all available replies
    while ser.in_waiting > 0:
        response = ser.readline().decode('utf-8', errors='ignore').strip()
        print(f"Received: {response}")

def set_volume(ser, volume):
    volume = max(0, min(255, volume))  # Clamp to 0-255
    send_command(ser, f'V{volume}')

def set_frequency(ser, frequency):
    send_command(ser, f'F{frequency}')

def main():
    parser = argparse.ArgumentParser(description="Set g_volume and g_settings.stimfreq via serial")
    parser.add_argument('-v', '--volume', type=int, help="Volume (0-255)", required=False)
    parser.add_argument('-f', '--frequency', type=int, help="Stimulation frequency (Hz)", required=False)
    parser.add_argument('-p', '--port', type=str, default=DEFAULT_SERIAL_PORT, help="Serial port (default COM17)")
    args = parser.parse_args()

    if args.volume is None and args.frequency is None:
        print("Error: You must specify at least --volume or --frequency")
        return

    # Open serial
    ser = serial.Serial(
        port=args.port,
        baudrate=BAUDRATE,
        timeout=TIMEOUT_SEC
    )

    if not ser.is_open:
        ser.open()

    # Wait a bit for Arduino reset
    time.sleep(2)

    # Send requested commands
    if args.volume is not None:
        set_volume(ser, args.volume)

    if args.frequency is not None:
        set_frequency(ser, args.frequency)

    ser.close()

if __name__ == '__main__':
    main()
