## **📝 Manual for `measure_latency.py`**

### **🎯 Purpose**

This script measures:

* **Response latency** of a serial device to three commands: `'1'`, `'0'`, and `'L'`.

* **Timing gaps** between those commands to assess jitter or delays in sequencing. The gaps are set at 1s.

---

### **⚙️ Requirements**

* Python 3.7+

`pyserial` installed  
 Install with:

 bash  
`pip install pyserial`

* A serial device connected that:

  * Listens on a COM port (e.g., COM8)

  * Responds to:

    * `'1'` with `'1'`

    * `'0'` with `'0'`

    * `'L'` with `'L'`

---

### **▶️ How to Run**

1. Open a terminal (e.g., PowerShell, CMD)

Navigate to the script folder:

 bash  
`cd C:\Users\pieter\Downloads`

2. 

Run the script:

 bash  
`python "measure_latency.py"`

3. 

---

### **🔧 Configuration**

You can change:

python

`measure_latency(port='COM8', samples=5)`

* `port='COM8'`: change to match your system (e.g., `'COM3'`, `'COM5'`)

* `samples=5`: number of test rounds (each round sends `'1'`, `'0'`, `'L'`)

---

### **📈 What It Measures**

For each command:

* The **round-trip latency** (time from send to correct response)

Between commands:

* **Gaps**:

  * From `'L'` to next `'1'`

  * From `'1'` to `'0'`

  * From `'0'` to `'L'`

All measurements are in **microseconds (µs)**.

---

### **🧾 Example Output**

`python.exe '.\measure _latency_jitter.py'`  
`Script is running.`  
`Attempting to open serial port: COM8 at 115200 baud`  
`Connected to COM8, beginning measurements...`  
`> Sending '1' for 1-1...`  
`Sample 1-1: 1133.90 µs`  
`> Sending '0' for 1-0...`  
`Sample 1-0: 410.40 µs`  
`> Sending 'L' for 1-L...`  
`Sample 1-L: 507.90 µs`  
`> Sending '1' for 2-1...`  
`Sample 2-1: 1265.20 µs`  
`> Sending '0' for 2-0...`  
`Sample 2-0: 376.80 µs`  
`> Sending 'L' for 2-L...`  
`Sample 2-L: 629.80 µs`  
`> Sending '1' for 3-1...`  
`Sample 3-1: 1294.40 µs`  
`> Sending '0' for 3-0...`  
`Sample 3-0: 393.30 µs`  
`> Sending 'L' for 3-L...`  
`Sample 3-L: 337.40 µs`  
`> Sending '1' for 4-1...`  
`Sample 4-1: 1145.30 µs`  
`> Sending '0' for 4-0...`  
`Sample 4-0: 583.50 µs`  
`> Sending 'L' for 4-L...`  
`Sample 4-L: 421.60 µs`  
`> Sending '1' for 5-1...`  
`Sample 5-1: 1386.10 µs`  
`> Sending '0' for 5-0...`  
`Sample 5-0: 343.50 µs`  
`> Sending 'L' for 5-L...`  
`Sample 5-L: 500.20 µs`

`=== Command '1' Statistics ===`  
`Samples: 5`  
`Average: 1244.98 µs`  
`Minimum: 1133.90 µs`  
`Maximum: 1386.10 µs`  
`Std Dev: 106.11 µs`

`=== Command '0' Statistics ===`  
`Samples: 5`  
`Average: 421.50 µs`  
`Minimum: 343.50 µs`  
`Maximum: 583.50 µs`  
`Std Dev: 93.87 µs`

`=== Command 'L' Statistics ===`  
`Samples: 5`  
`Average: 479.38 µs`  
`Minimum: 337.40 µs`  
`Maximum: 629.80 µs`  
`Std Dev: 108.82 µs`

`=== Gap L→1 Statistics ===`  
`Samples: 4`  
`Average: 1000576.65 µs`  
`Minimum: 1000298.20 µs`  
`Maximum: 1000821.80 µs`  
`Std Dev: 271.42 µs`

`=== Gap 1→0 Statistics ===`  
`Samples: 5`  
`Average: 1000593.14 µs`  
`Minimum: 1000338.60 µs`  
`Maximum: 1000709.80 µs`  
`Std Dev: 152.56 µs`

`=== Gap 0→L Statistics ===`  
`Samples: 5`  
`Average: 1000566.94 µs`  
`Minimum: 1000154.90 µs`  
`Maximum: 1001069.50 µs`  
`Std Dev: 374.92 µs`  
`PS C:\Users\pieter\Downloads>`  
---

### **🧠 Notes**

* The device must echo back the expected single character (`'1'`, `'0'`, `'L'`) immediately.

* Make sure **no other program** is using the COM port.

* You can test manually using tools like **Tera Term** or **PuTTY**.

