import time
import serial
import statistics

def measure_latency(port='COM17', baudrate=115200, samples=5):
    latencies_1 = []
    latencies_0 = []
    
    try:
        with serial.Serial(port, baudrate, timeout=1) as ser:
            print(f"Connected to {port}, beginning measurements...")
            
            for i in range(samples):
                # Clear buffers before each test pair
                ser.reset_input_buffer()
                ser.reset_output_buffer()
                
                # --- Test '1' command ---
                start_time_1 = time.perf_counter_ns()
                ser.write(b'1\n')  # Send '1' with newline
                
                # Wait for acknowledgment
                response_1 = None
                while (time.perf_counter_ns() - start_time_1) < 1_000_000_000:  # 1s timeout
                    if ser.in_waiting:
                        response_1 = ser.read(1).decode()
                        if response_1 == '1':
                            latency_1 = (time.perf_counter_ns() - start_time_1) / 1000
                            latencies_1.append(latency_1)
                            print(f"Sample {i+1}-1: {latency_1:.2f} µs")
                            break
                
                if response_1 != '1':
                    print(f"Sample {i+1}-1: Timeout or invalid response")
                
                # --- Test '0' command ---
                time.sleep(1)  # Wait 1 second between commands
                
                start_time_0 = time.perf_counter_ns()
                ser.write(b'0\n')  # Send '0' with newline
                
                # Wait for acknowledgment
                response_0 = None
                while (time.perf_counter_ns() - start_time_0) < 1_000_000_000:  # 1s timeout
                    if ser.in_waiting:
                        response_0 = ser.read(1).decode()
                        if response_0 == '0':
                            latency_0 = (time.perf_counter_ns() - start_time_0) / 1000
                            latencies_0.append(latency_0)
                            print(f"Sample {i+1}-0: {latency_0:.2f} µs")
                            break
                
                if response_0 != '0':
                    print(f"Sample {i+1}-0: Timeout or invalid response")
                
                time.sleep(5)  # 5 second delay between sample pairs
    
    except serial.SerialException as e:
        print(f"Serial error: {e}")
    
    # Print statistics for both commands
    def print_stats(label, latencies):
        if latencies:
            print(f"\n=== {label} Statistics ===")
            print(f"Samples: {len(latencies)}")
            print(f"Average: {statistics.mean(latencies):.2f} µs")
            print(f"Minimum: {min(latencies):.2f} µs")
            print(f"Maximum: {max(latencies):.2f} µs")
            print(f"Std Dev: {statistics.stdev(latencies):.2f} µs")
        else:
            print(f"\nNo successful measurements for {label}")
    
    print_stats("Command '1'", latencies_1)
    print_stats("Command '0'", latencies_0)

if __name__ == "__main__":
    measure_latency(samples=10)  # Measures 6 pairs of commands