import socket
import struct
import numpy as np

HOST = '0.0.0.0'
PORT = 12345
CHUNK_SIZE = 8 * 251 * 3  # Same size
CHUNKS_PER_SECOND = 25
TOTAL_SECONDS = 10
TOTAL_CHUNKS = TOTAL_SECONDS * CHUNKS_PER_SECOND

def receive_data():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, PORT))
        s.listen(1)
        print(f"Listening on {HOST}:{PORT}...")

        conn, addr = s.accept()
        with conn:
            print(f"Connected by {addr}")

            full_arr1 = []
            full_arr2 = []
            full_arr3 = []

            for second in range(TOTAL_SECONDS):
                for chunk in range(TOTAL_CHUNKS):
                    data = b''
                    while len(data) < CHUNK_SIZE:
                        packet = conn.recv(CHUNK_SIZE - len(data))
                        if not packet:
                            print("Connection closed early.")
                            break
                        data += packet

                    if len(data) < CHUNK_SIZE:
                        break  # Exit early if incomplete

                    values = struct.unpack('753d', data)
                    full_arr1.extend(values[:251])
                    full_arr2.extend(values[251:502])
                    full_arr3.extend(values[502:])
                    print(f"Received chunk {chunk+1}/{TOTAL_CHUNKS}")

            # Convert final arrays
            arr1 = np.array(full_arr1)
            arr2 = np.array(full_arr2)
            arr3 = np.array(full_arr3)

            print(f"Final shapes: {arr1.shape}, {arr2.shape}, {arr3.shape}")
            print(f"Sample values: {arr1[:5]}, {arr2[:5]}, {arr3[:5]}")

if __name__ == "__main__":
    receive_data()
