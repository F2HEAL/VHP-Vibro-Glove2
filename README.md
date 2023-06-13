# VHP-Vibro-Glove2

This is a driver for a home-brew attempt at a [vibrating glove](https://stanmed.stanford.edu/vibrating-glove-reorganize-neurons-parkinsons/).

## Hardware 

This software was tested on the hardware developed in the excellent [Audio-to-tactile](https://github.com/google/audio-to-tactile) project. 

## Software

### Building

The software can be built in two ways:
1. Arduino IDE/CLI for using 
2. On local PC for testing

#### Arduino IDE/CLI

Follow [these instructions](https://github.com/F2HEAL/VHP-Vibro-Glove/blob/main/README.md) to prepare your Arduino environment.

Afterwards, you should be able to compile and upload the sketch in [/VHP-Vibro-Glove](/VHP-Vibro-Glove2)

      $ arduino-cli compile --fqbn adafruit:nrf52:feather52840 -v VHP-Vibro-Glove2/ 
      ...
      adafruit-nrfutil dfu genpkg --dev-type 0x0052 --sd-req 0x00B6 --application /tmp/arduino/sketches/4AEB...
      Zip created at /tmp/arduino/sketches/4AEB02B3AD62FE558102135EA5FA23D8/VHP-Vibro-Glove2.ino.zip
      ...
      Sketch uses 91348 bytes (11%) of program storage space. Maximum is 815104 bytes.
      Global variables use 17040 bytes (7%) of dynamic memory, leaving 220528 bytes for local variables. Maximum is 237568 bytes.

      $ arduino-cli upload --fqbn adafruit:nrf52:feather52840 -v VHP-Vibro-Glove2/ --port /dev/ttyACM0 
      Performing 1200-bps touch reset on serial port /dev/ttyACM0
      Waiting for upload port...
      No upload port found, using /dev/ttyACM0 as fallback
      "adafruit-nrfutil" --verbose dfu serial -pkg "/tmp/arduino/sketches/4AEB02B3AD62FE558102135EA5FA23D8/VHP-Vibro-Glove2.ino.zip" -p /dev/ttyACM0 -b 115200 --singlebank
      Upgrading target on /dev/ttyACM0 with DFU package /tmp/arduino/sketches/4AEB02B3AD62FE558102135EA5FA23D8/VHP-Vibro-Glove2.ino.zip. Flow control is disabled, Single bank, Touch disabled
      Opened serial port /dev/ttyACM0
      Starting DFU upgrade of type 4, SoftDevice size: 0, bootloader size: 0, application size: 91356
      Sending DFU start packet
      Sending DFU init packet
      Sending firmware file
      ########################################
      ########################################
      ########################################
      ########################################
      ###################
      Activating new firmware
      
      DFU upgrade took 5.993805170059204s
      Device programmed.

#### Local build

To build locally [CMake](https://cmake.org/) and a working C++ compiler are required: [g++](https://gcc.gnu.org/), [clang++](https://clang.llvm.org/)

    $ mkdir build
    $ cd build/
    $ cmake ..
    -- The C compiler identification is GNU 12.3.1
    -- The CXX compiler identification is GNU 12.3.1
    -- Detecting C compiler ABI info
    -- Detecting C compiler ABI info - done
    -- Check for working C compiler: /usr/bin/cc - skipped
    -- Detecting C compile features
    -- Detecting C compile features - done
    -- Detecting CXX compiler ABI info
    -- Detecting CXX compiler ABI info - done
    -- Check for working CXX compiler: /usr/bin/c++ - skipped
    -- Detecting CXX compile features
    -- Detecting CXX compile features - done
    -- Configuring done (0.5s)
    -- Generating done (0.0s)
    -- Build files have been written to: /home/wgodefro/src/f2heal/VHP-Vibro-Glove2/build
    $ make
    [ 25%] Building CXX object CMakeFiles/sstream-test.dir/tests/SStream-test.cpp.o
    [ 50%] Linking CXX executable sstream-test
    [ 50%] Built target sstream-test
    [ 75%] Building CXX object CMakeFiles/samplecache-test.dir/tests/SampleCache-test.cpp.o
    [100%] Linking CXX executable samplecache-test
    [100%] Built target samplecache-test

### Internals

The software will produce a waveform in a random sequence on the different channels according to the settings passed to **SStream**. The default settings in [VHP-Vibro-Glove2.ino](VHP-Vibro-Glove2/VHP-Vibro-Glove2.ino).

These parameters are defined and documented in [SStream.hpp](VHP-Vibro-Glove2/SStream.hpp) and illustrated below.

![image](https://github.com/F2HEAL/VHP-Vibro-Glove2/assets/18469570/2a48a60a-b9d9-407f-b9bf-ead3eb6b7bac)

![image](https://github.com/F2HEAL/VHP-Vibro-Glove2/assets/18469570/fc01761d-4320-4044-b50b-c7768205cf2b)

