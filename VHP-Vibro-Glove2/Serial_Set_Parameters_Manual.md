# Serial Volume and Frequency Setter

This document explains the usage and functionality of the provided Python script for setting volume and frequency via a serial connection.

## Overview

This script connects to a serial device (e.g., an Arduino) and sends commands to update the volume (`g_volume`) and stimulation frequency (`g_settings.stimfreq`).

---

## Script: Volume and Frequency Setter

```python
import serial
import time
import argparse

# Default settings
DEFAULT_SERIAL_PORT = 'COM17'
BAUDRATE = 115200
TIMEOUT_SEC = 1
```
Defines serial connection parameters.

### Functions

#### `send_command(ser, command)`
- Sends a command with a newline (`\n`) over serial.
- Waits briefly and prints all incoming responses.

#### `set_volume(ser, volume)`
- Sends a volume setting command.
- Volume is clamped between 0 and 255.

#### `set_frequency(ser, frequency)`
- Sends a frequency setting command.

---

## `main()` Function

- Parses command-line arguments:
  - `--volume` (`-v`) to set volume.
  - `--frequency` (`-f`) to set stimulation frequency.
  - `--port` (`-p`) to specify the serial port (default: `COM17`).
- Validates that at least one setting is provided.
- Opens the serial port and waits 2 seconds (for device reset).
- Sends the selected volume and/or frequency.
- Closes the serial connection.

### Example Usage

```bash
python set_volume_frequency.py --volume 128 --frequency 250
python set_volume_frequency.py -v 100
python set_volume_frequency.py -f 200 -p /dev/ttyUSB0
```

---

## Serial Command Format

| Command Sent | Effect                          |
|:------------:|:-------------------------------:|
| `V<value>`   | Set volume (0–255)               |
| `F<value>`   | Set stimulation frequency (Hz)  |

---

## Notes

- Ensure the device expects and correctly handles `V<value>` and `F<value>` commands.
- Add a small delay after opening the port to allow device reset.
- The script reads and prints all responses from the device for verification.

---

# 📋 License

Free to use and modify for your projects!
