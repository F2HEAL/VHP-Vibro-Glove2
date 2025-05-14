import time
import serial
import statistics

def measure_latency(port='COM8', baudrate=115200, samples=5):
    latencies_1 = []
    latencies_0 = []
    latencies_L = []
    gaps_1_to_0 = []
    gaps_0_to_L = []
    gaps_L_to_1 = []

    def send_and_measure(ser, cmd, expected_resp, sample_label):
        print(f"> Sending '{cmd}' for {sample_label}...")
        start_time = time.perf_counter_ns()
        ser.write(f"{cmd}\n".encode())

        while (time.perf_counter_ns() - start_time) < 1_000_000_000:  # 1 second timeout
            if ser.in_waiting:
                raw = ser.read(1)
                try:
                    response = raw.decode()
                    if response == expected_resp:
                        latency = (time.perf_counter_ns() - start_time) / 1000  # microseconds
                        print(f"Sample {sample_label}: {latency:.2f} µs")
                        return latency, time.perf_counter_ns()
                except UnicodeDecodeError:
                    continue
        print(f"Sample {sample_label}: Timeout or invalid response")
        return None, time.perf_counter_ns()

    try:
        print(f"Attempting to open serial port: {port} at {baudrate} baud")
        with serial.Serial(port, baudrate, timeout=1) as ser:
            print(f"Connected to {port}, beginning measurements...")

            prev_end_L = None

            for i in range(samples):
                ser.reset_input_buffer()
                ser.reset_output_buffer()

                # --- Command '1' ---
                start_1 = time.perf_counter_ns()
                if prev_end_L is not None:
                    gaps_L_to_1.append((start_1 - prev_end_L) / 1000)
                latency_1, end_1 = send_and_measure(ser, '1', '1', f"{i+1}-1")
                if latency_1 is not None:
                    latencies_1.append(latency_1)
                time.sleep(1)

                # --- Command '0' ---
                gap_1_to_0 = (time.perf_counter_ns() - end_1) / 1000
                gaps_1_to_0.append(gap_1_to_0)
                latency_0, end_0 = send_and_measure(ser, '0', '0', f"{i+1}-0")
                if latency_0 is not None:
                    latencies_0.append(latency_0)
                time.sleep(1)

                # --- Command 'L' ---
                gap_0_to_L = (time.perf_counter_ns() - end_0) / 1000
                gaps_0_to_L.append(gap_0_to_L)
                latency_L, end_L = send_and_measure(ser, 'L', 'L', f"{i+1}-L")
                if latency_L is not None:
                    latencies_L.append(latency_L)
                prev_end_L = end_L

                time.sleep(1)

    except serial.SerialException as e:
        print(f"Serial error: {e}")
    except Exception as e:
        print(f"General error: {e}")

    def print_stats(label, values):
        if values:
            print(f"\n=== {label} Statistics ===")
            print(f"Samples: {len(values)}")
            print(f"Average: {statistics.mean(values):.2f} µs")
            print(f"Minimum: {min(values):.2f} µs")
            print(f"Maximum: {max(values):.2f} µs")
            print(f"Std Dev: {statistics.stdev(values):.2f} µs")
        else:
            print(f"\nNo data for {label}")

    # Latency stats
    print_stats("Command '1'", latencies_1)
    print_stats("Command '0'", latencies_0)
    print_stats("Command 'L'", latencies_L)

    # Inter-command timing
    print_stats("Gap L→1", gaps_L_to_1)
    print_stats("Gap 1→0", gaps_1_to_0)
    print_stats("Gap 0→L", gaps_0_to_L)

if __name__ == "__main__":
    print("Script is running.")
    measure_latency(port='COM8', samples=5)
