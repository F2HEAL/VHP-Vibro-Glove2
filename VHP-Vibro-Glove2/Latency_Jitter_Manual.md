# Latency and Jitter Measurement Script

This document explains the usage and functionality of the provided Python script `measure_latency_jitter.py`.

## Overview

The script measures the **latency** and **jitter** of command acknowledgments sent over a serial port.

It sends alternating '1' and '0' commands to the connected device and measures the time taken to receive the correct response.

---

## How It Works

1. Open a serial connection to the specified port at a given baud rate.
2. Send a `'1'` command (`1\n`), and measure the time until a `'1'` is received back.
3. Send a `'0'` command (`0\n`), and measure the time until a `'0'` is received back.
4. Repeat the process for a specified number of samples.
5. Compute statistics: average latency, minimum, maximum, and standard deviation for each command type.

---

## Script: `measure_latency_jitter.py`

```python
def measure_latency(port='COM17', baudrate=115200, samples=5):
    # Measures latency and jitter for '1' and '0' serial commands
```

### Parameters:

| Name      | Type    | Description                              |
|-----------|---------|------------------------------------------|
| `port`    | String  | Serial port name (e.g., `COM17` or `/dev/ttyUSB0`) |
| `baudrate`| Integer | Serial baud rate (default: 115200)        |
| `samples` | Integer | Number of sample pairs to measure        |

### Measurement Method:

- Each '1' and '0' is measured separately.
- The script uses `time.perf_counter_ns()` for high-precision timing.
- A 1-second timeout is set for responses.
- 5 seconds of delay is introduced between sample pairs.

### Output:

- Individual latencies printed to the console (in microseconds µs).
- Summary statistics printed at the end for both commands.

### Example Output:

```text
Sample 1-1: 415.50 µs
Sample 1-0: 398.20 µs
...
=== Command '1' Statistics ===
Samples: 10
Average: 410.45 µs
Minimum: 398.20 µs
Maximum: 425.80 µs
Std Dev: 8.25 µs
```

---

## Notes

- Ensure the device on the other side echoes back '1' and '0' properly for accurate measurements.
- Script automatically handles buffer resets (`reset_input_buffer()`, `reset_output_buffer()`).
- Adjust `samples`, `port`, and `baudrate` as needed.

---

# 📋 License

Feel free to use and modify for testing and diagnostics!
