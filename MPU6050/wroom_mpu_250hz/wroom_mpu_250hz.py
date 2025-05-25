import socket
import numpy as np

# Configuration
SERVER_IP = '0.0.0.0'  # Listen on all interfaces
SERVER_PORT = 12345
BUFFER_SIZE = 4096

def receive_data():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((SERVER_IP, SERVER_PORT))
        s.listen()
        print(f"Listening on {SERVER_IP}:{SERVER_PORT}")
        conn, addr = s.accept()
        with conn:
            print(f"Connected by {addr}")
            data = b''
            while True:
                chunk = conn.recv(BUFFER_SIZE)
                if not chunk:
                    break
                data += chunk
            return data

def process_data(raw_data):
    # Convert bytes to float array (4 bytes per float)
    float_array = np.frombuffer(raw_data, dtype=np.float16)
    
    # Verify data size (should be 3000 floats: 1000 samples * 3 axes)
    if len(float_array) != 3000:
        print(f"Warning: Received {len(float_array)} samples (expected 3000)")
    
    # Reconstruct axis blocks (10x [100x + 100y + 100z])
    x = np.concatenate([float_array[i*300 : i*300+100] for i in range(10)])
    y = np.concatenate([float_array[i*300+100 : i*300+200] for i in range(10)])
    z = np.concatenate([float_array[i*300+200 : i*300+300] for i in range(10)])
    
    return x, y, z

# Main execution
raw_data = receive_data()
x, y, z = process_data(raw_data)

# Print first 10 samples of each axis
print("\nAccelerometer Data (g):")
print(f"{'X':>8} {'Y':>8} {'Z':>8}")
for i in range(10):
    print(f"{x[i]:8.3f} {y[i]:8.3f} {z[i]:8.3f}")

# Print statistics
print("\nData Statistics:")
print(f"X: Min {x.min():.3f}g | Max {x.max():.3f}g | Mean {x.mean():.3f}g")
print(f"Y: Min {y.min():.3f}g | Max {y.max():.3f}g | Mean {y.mean():.3f}g")
print(f"Z: Min {z.min():.3f}g | Max {z.max():.3f}g | Mean {z.mean():.3f}g")
