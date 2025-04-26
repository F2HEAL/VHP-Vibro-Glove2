# Serial Command Interface

This document explains how the Arduino firmware handles serial commands for controlling streaming and adjusting parameters.

## Overview

The system listens for incoming serial data. It buffers characters until a newline (`\n`) or carriage return (`\r`) is detected, then processes the command.

Commands can be either:

- **Single-character** commands: trigger basic actions.
- **Multi-character** commands: configure numeric parameters (e.g., volume or frequency).

---

## `loop()` Function

```cpp
void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();

    if (cmd == '\n' || cmd == '\r') {
      // End of a command
      if (serialBuffer.length() > 0) {
        processSerialCommand(serialBuffer);
        serialBuffer = "";
      }
    }
    else {
      serialBuffer += cmd;
    }
  }
}
```

**Summary:**

- Reads incoming characters and stores them in `serialBuffer`.
- When a newline or carriage return is received:
  - Calls `processSerialCommand(serialBuffer)`.
  - Clears the buffer for the next command.

---

## `processSerialCommand()` Function

```cpp
void processSerialCommand(String cmd) {
  if (cmd.length() == 0) return;

  if (cmd.length() == 1) {
    char c = cmd.charAt(0);
    
    if (c == '1') {
      StartStream();
      Serial.write('1');
    }
    else if (c == '0') {
      StopStream();
      Serial.write('0');
    }
    else if (c == 'T') {
      ToggleStream();
      Serial.write('T');
    }
    else if (c == 'L') {
      Serial.write('L');
    }
    else {
      Serial.print("Unknown: ");
      Serial.println(c);
    }
  }
  else {
    // Multi-character command
    char commandType = cmd.charAt(0);
    int value = cmd.substring(1).toInt();

    switch (commandType) {
      case 'V':
        g_volume = constrain(value, 0, 255);
        Serial.print("Volume set to ");
        Serial.println(g_volume);
        break;

      case 'F':
        g_settings.stimfreq = value;
        Serial.print("Frequency set to ");
        Serial.println(g_settings.stimfreq);
        break;

      default:
        Serial.print("Unknown command: ");
        Serial.println(cmd);
        break;
    }
  }
}
```

---

## Single-Character Commands

| Command | Action                              | Response Sent Back |
| ------- | ----------------------------------- | ------------------ |
| `'1'`   | Start streaming (`StartStream()`)   | `'1'`              |
| `'0'`   | Stop streaming (`StopStream()`)     | `'0'`              |
| `'T'`   | Toggle streaming (`ToggleStream()`) | `'T'`              |
| `'L'`   | Echo `'L'` back (no action)         | `'L'`              |
| Unknown | Prints `Unknown: <character>`       | —                  |

---

## Multi-Character Commands

| Command Format | Meaning                   | Action                                |
| -------------- | ------------------------- | ------------------------------------- |
| `V<number>`    | Set volume                | `g_volume = constrain(value, 0, 255)` |
| `F<number>`    | Set stimulation frequency | `g_settings.stimfreq = value`         |

**Examples:**

- `V128\n` → Sets volume to 128
- `F250\n` → Sets frequency to 250 Hz

Unknown commands print a message like:

```text
Unknown command: X123
```

---

## Notes

- **StartStream()**, **StopStream()**, and **ToggleStream()** must be defined elsewhere.
- Commands are **case-sensitive**.
- Jitter between serial characters does not affect command interpretation as long as the end marker (`\n` or `\r`) is received.
- Numeric values are parsed as integers.
- The system provides basic feedback over Serial to confirm actions.

---

## Example Serial Commands

| Sent     | Effect                   |
| -------- | ------------------------ |
| `1\n`    | Start streaming.         |
| `0\r`    | Stop streaming.          |
| `T\n`    | Toggle streaming state.  |
| `L\n`    | Echo `'L'` back.         |
| `V200\n` | Set volume to 200.       |
| `F50\n`  | Set frequency to 50 Hz.  |
| `X\n`    | Unknown command warning. |

---

# 📋 License

Feel free to use and modify for your own projects!
