import socket
import struct
import numpy as np
import matplotlib.pyplot as plt

# Parameters for network and data
HOST = '0.0.0.0'
PORT = 12345
CHUNK_SIZE = 4 * 300  # 300 floats * 4 bytes per float = 1200 bytes per chunk

# Sampling frequency and time step for integration and FFT (assumed 50 Hz)
fs = 500  
dt = 1 / fs

def compute_fft(data, dt):
    """Compute FFT of the data and return positive frequencies and scaled magnitudes."""
    N = len(data)
    fft_values = np.fft.fft(data)
    frequencies = np.fft.fftfreq(N, d=dt)
    pos_mask = frequencies > 0  # Only positive frequencies
    return frequencies[pos_mask], (2.0 / N) * np.abs(fft_values[pos_mask])

def calculate_velocity(acc, dt):
    """Compute velocity using trapezoidal integration of acceleration data.
       Assumes initial velocity is zero.
    """
    N = len(acc)
    vel = np.zeros(N)
    for i in range(1, N):
        # Trapezoidal integration: v[i] = v[i-1] + (acc[i] + acc[i-1])*dt/2
        vel[i] = vel[i-1] + (acc[i] + acc[i-1]) * dt / 2.0
    return vel

def update_plots(arr1, arr2, arr3, fig, axs):
    """Update the 4 subplots with the new acceleration arrays (arr1, arr2, arr3).
       This includes time series, FFTs, and velocities.
    """
    # Compute velocities for each axis
    vel_x = calculate_velocity(arr1, dt)
    vel_y = calculate_velocity(arr2, dt)
    vel_z = calculate_velocity(arr3, dt)
    
    # Compute FFTs for acceleration
    freq_ax, fft_ax = compute_fft(arr1, dt)
    freq_ay, fft_ay = compute_fft(arr2, dt)
    freq_az, fft_az = compute_fft(arr3, dt)
    
    # Compute FFTs for velocity
    freq_vx, fft_vx = compute_fft(vel_x, dt)
    freq_vy, fft_vy = compute_fft(vel_y, dt)
    freq_vz, fft_vz = compute_fft(vel_z, dt)
    
    # Clear and update subplot 1: Acceleration time series
    axs[0, 0].cla()
    axs[0, 0].plot(arr1, 'r-', label='Acc X')
    axs[0, 0].plot(arr2, 'g-', label='Acc Y')
    axs[0, 0].plot(arr3, 'b-', label='Acc Z')
    axs[0, 0].set_title("Acceleration Time Series")
    axs[0, 0].set_xlabel("Sample Number")
    axs[0, 0].set_ylabel("Acceleration (g)")
    axs[0, 0].legend()
    
    # Clear and update subplot 2: FFT of acceleration
    axs[0, 1].cla()
    axs[0, 1].plot(freq_ax, fft_ax, 'r-', label='Acc X FFT')
    axs[0, 1].plot(freq_ay, fft_ay, 'g-', label='Acc Y FFT')
    axs[0, 1].plot(freq_az, fft_az, 'b-', label='Acc Z FFT')
    axs[0, 1].set_title("Acceleration FFT")
    axs[0, 1].set_xlabel("Frequency (Hz)")
    axs[0, 1].set_ylabel("Magnitude")
    axs[0, 1].legend()
    
    # Clear and update subplot 3: Velocity time series
    axs[1, 0].cla()
    axs[1, 0].plot(vel_x, 'r-', label='Vel X')
    axs[1, 0].plot(vel_y, 'g-', label='Vel Y')
    axs[1, 0].plot(vel_z, 'b-', label='Vel Z')
    axs[1, 0].set_title("Velocity Time Series")
    axs[1, 0].set_xlabel("Sample Number")
    axs[1, 0].set_ylabel("Velocity (units/s)")
    axs[1, 0].legend()
    
    # Clear and update subplot 4: FFT of velocity
    axs[1, 1].cla()
    axs[1, 1].plot(freq_vx, fft_vx, 'r-', label='Vel X FFT')
    axs[1, 1].plot(freq_vy, fft_vy, 'g-', label='Vel Y FFT')
    axs[1, 1].plot(freq_vz, fft_vz, 'b-', label='Vel Z FFT')
    axs[1, 1].set_title("Velocity FFT")
    axs[1, 1].set_xlabel("Frequency (Hz)")
    axs[1, 1].set_ylabel("Magnitude")
    axs[1, 1].legend()
    
    # Draw updated plots
    fig.canvas.draw()
    fig.canvas.flush_events()
    # A short pause to allow the GUI event loop to update the plots
    plt.pause(0.001)

def receive_data(fig, axs):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, PORT))
        s.listen()
        print(f"Listening on {HOST}:{PORT}...")

        while True:
            conn, addr = s.accept()
            print(f"Connected by {addr}")
            
            with conn:
                while True:
                    # Create new storage for a complete cycle of 10 chunks.
                    full_arr1 = []
                    full_arr2 = []
                    full_arr3 = []
                    chunk_count = 0
                    
                    # Receive exactly 10 chunks and append each axis data to the corresponding array.
                    while chunk_count < 10:
                        data = b''
                        # Accumulate data until a full chunk is received.
                        while len(data) < CHUNK_SIZE:
                            packet = conn.recv(CHUNK_SIZE - len(data))
                            if not packet:
                                # Connection closed or no more data.
                                break
                            data += packet

                        # Check if we received an incomplete chunk.
                        if len(data) < CHUNK_SIZE:
                            print("Incomplete data received or connection closed during cycle.")
                            break

                        # Unpack the chunk into 300 floats (100 per axis).
                        values = struct.unpack('300f', data)
                        axis1 = values[:100]
                        axis2 = values[100:200]
                        axis3 = values[200:]
                        
                        # Append the data to the full arrays.
                        full_arr1.extend(axis1)
                        full_arr2.extend(axis2)
                        full_arr3.extend(axis3)
                        
                        chunk_count += 1
                        print(f"Received chunk {chunk_count}/10")

                    # If an incomplete cycle was detected, exit this connection cycle.
                    if chunk_count < 10:
                        print("Cycle incomplete. Ending this connection cycle.")
                        break

                    # Convert the complete cycle lists into numpy arrays (each with 1000 samples).
                    arr1 = np.array(full_arr1)
                    arr2 = np.array(full_arr2)
                    arr3 = np.array(full_arr3)
                    
                    print("Completed cycle of 10 chunks:")
                    print(f"Final shapes: {arr1.shape}, {arr2.shape}, {arr3.shape}")
                    print(f"Sample values (first 5): {arr1[:5]}, {arr2[:5]}, {arr3[:5]}")
                    
                    # Update the live plots with the new 1000-sample arrays.
                    update_plots(arr1, arr2, arr3, fig, axs)
                    
            print("Connection closed. Waiting for new connection...")

if __name__ == "__main__":
    # Enable interactive mode so the plot updates without blocking.
    plt.ion()
    # Create the figure and a 2x2 grid of subplots once.
    fig, axs = plt.subplots(2, 2, figsize=(12, 10))
    plt.tight_layout()
    
    # Start the receiver which will update the same figure continuously.
    receive_data(fig, axs)
