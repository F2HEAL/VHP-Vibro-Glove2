# BLE 

## Parameters

### Parameters in (Configurables)

#### At any moment

- volume      (uint8)
- toggle state, start/stop
- test channel 1-8 [8 bool]


#### Before start

- 8 channels       (bool)
- stimfreq         (uint32)
- stimdur          (uint32)
- cycleperiod      (uint32)
- pauzecycleperiod (uint32)
- pauzedcycles     (uint32)
- jitter           (uint32)
- test_mode        (bool)

### Parameters out (status)

- current status batch
  - battery voltage
  - temperature
  - running (since)
  - software version
  - (log output)

#### The configurables

To read out the current status:

- volume (uint8)

- current settigns batch
  - 8 channels       (bool)
  - stimfreq         (uint32)
  - stimdur          (uint32)
  - cycleperiod      (uint32)
  - pauzecycleperiod (uint32)
  - pauzedcycles     (uint32)
  - jitter           (uint32)
  - test_mode        (bool)
  



## Implementation steps

- write message.hpp
- write ble_com.hpp
- rewrite VHP-Vibro-Glove2.ino 
  - include BLE
  - start/stop 
  - volume change
  - configurable parameters (if not running)
