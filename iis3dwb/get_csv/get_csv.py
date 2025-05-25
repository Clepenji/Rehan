import serial

# Replace 'COM3' with your port (Windows 'COMx', Linux/Mac '/dev/ttyUSB0' or similar)
SERIAL_PORT = 'COM3'
BAUD_RATE = 115200
OUTPUT_FILE = 'accelerometer_data.csv'

with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1) as ser, open(OUTPUT_FILE, 'w', newline='') as f:
    print("Recording data. Press Ctrl+C to stop.")
    while True:
        try:
            line = ser.readline().decode('utf-8').strip()
            if line:
                f.write(line + '\n')
                print(line)
        except KeyboardInterrupt:
            print("Stopped recording.")
            break
