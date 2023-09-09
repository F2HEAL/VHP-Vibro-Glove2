# VHP-Vibro-Glove2

Open Source design for Vibrotactile Coordinated Reset (vCR) Gloves.

## Disclaimer

This product is not a medical device and is not intended to diagnose, treat, cure, or prevent any disease or medical condition. It should not be used as a substitute for professional medical advice, diagnosis, or treatment. Always consult with a qualified healthcare professional before making any decisions related to your health or using this product in conjunction with a medical treatment plan.

## Hardware 

We used the board developed in the [Google Audio-to-tactile](https://github.com/google/audio-to-tactile) project (Thanks!). Instructions on how to obtain this board are provided [here](https://github.com/google/audio-to-tactile/blob/main/extras/doc/hardware/index.md).

We used 8 [TECTONIC TEAX09C005-8 Audio Exciters](https://www.tectonicaudiolabs.com/product/teax09c005-8/) in a 3D printed housing. Latest version [Gen13.stl](doc/Gen13.stl) with instructions on assembly [here](https://bb.f2heal.com/viewtopic.php?p=11#p11).

![image](https://github.com/F2HEAL/VHP-Vibro-Glove2/assets/18469570/78e032f1-99f6-40ed-86ab-5e5dde163579)

![image](https://github.com/F2HEAL/VHP-Vibro-Glove2/assets/18469570/6d37423f-a8d8-489d-8761-dce0398e5a9a)

## Software

### Building

The software can be built in two ways:
1. Arduino IDE/CLI for using 
2. On local PC for testing

#### Arduino IDE/CLI

Follow [these instructions](https://github.com/F2HEAL/VHP-Vibro-Glove/blob/main/README.md) to prepare your Arduino environment.

Afterwards, you should be able to compile and upload the sketch in [VHP-Vibro-Glove](/VHP-Vibro-Glove2)

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

The software will produce a waveform in a random sequence on the different channels according to the settings passed to **SStream**.

These parameters are defined and documented in [SStream.hpp](VHP-Vibro-Glove2/SStream.hpp) and illustrated below.

![image](https://github.com/F2HEAL/VHP-Vibro-Glove2/assets/18469570/2a48a60a-b9d9-407f-b9bf-ead3eb6b7bac)

![image](https://github.com/F2HEAL/VHP-Vibro-Glove2/assets/18469570/fc01761d-4320-4044-b50b-c7768205cf2b)

 The default settings in [VHP-Vibro-Glove2.ino](VHP-Vibro-Glove2/VHP-Vibro-Glove2.ino) provide with a balanced default settings:
 * 8 channels
 * Samplerate 46875Hz, long story, tied to SetUpsamplingFactor(1), only touch it if you know what you're doing ;-)
 * Stimulation frequency 250Hz

      * This means that one stimulation sine cycle takes  46875Hz / 250Hz = 187.5 samples
 * Stimulation duration 100ms
      * This means that one stimulation period takes 4687.5 samples

* Cycle period 1332ms. This leaves 1332ms / 8  = 166.5ms as a maximum for stimulation duration. With a stimulation duration of 100ms this leaves 166.5 - 100 = 66.5ms of silence before the next channel starts

* Pauze-cyle period 5 & Pauzed cycles 2 : For every 5 cycles 2 will be pauzed, total silence on all channels. So on 5 * 1332ms = 6660ms there will 2 * 1332ms = 2664ms of silence
* Jitter 23.5% : This is 23.5% of 1332ms / 8 or 39.1ms, so well below the 66.5ms of silence calculated above


**WARNING:** it is up to the user to ensure that the configured values make sense, such as that the Jitter is not higher than silence after the stimulation. The [settings.ods](doc/settings.ods) spreadsheet can be used to verify your settings.
