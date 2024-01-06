#include <stdint.h>

#ifndef BOARDDEFS_HPP_
#define BOARDDEFS_HPP_

#define kLedPinBlue 45
#define kLedPinGreen 36
#define kThermistorPin 3
#define kCurrentAmpEnable 42
#define kPdmClockPin 6
#define kPdmDataPin 7
#define kPdmSelectPin 26
#define kTactileSwitchPin 34

//With the upgrade to adafruit:nrf52 1.6.0 the existing approach via
//UI.hpp no longer worked. It is replaced by a call to attachInterrupt
//from the NRF SDK. 
#define kTactileSwitchPin_nrf 7 


// Output sequence for board Apollo84 hardware
//int order_pairs[8] = {0, 3, 4, 5, 11, 9, 8, 6};

// Output sequence for Godef Hardware
const uint16_t order_pairs[8] = {4, 5, 6, 7, 8, 9, 10, 11}; 


namespace audio_tactile {

    // Number of ADC samples per buffer.
    constexpr int kAdcDataSize = 64;
    // Number of PWM samples for each channel per buffer.
    constexpr int kNumPwmValues = 8;
    // Number of PWM channels.
    constexpr int kNumTotalPwm = 12;
    // Max length of a TactilePattern pattern string, not including null terminator.
    constexpr int kMaxTactilePatternLength = 15;
    // Max length of a device name string, not including null terminator.
    constexpr int kMaxDeviceNameLength = 16;

    // Constants for mic input selection.
    enum class InputSelection { kAnalogMic, kPdmMic };
    
}  // namespace audio_tactile


#endif
