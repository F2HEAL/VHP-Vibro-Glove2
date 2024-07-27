#include <stdint.h>

#ifndef BOARDDEFS_HPP_
#define BOARDDEFS_HPP_



// Output sequence for board Apollo84 hardware
//int order_pairs[8] = {0, 3, 4, 5, 11, 9, 8, 6};

// Output sequence for Godef Hardware
const uint16_t order_pairs[8] = {4, 5, 6, 7, 8, 9, 10, 11}; 


namespace audio_tactile {

    // PIN definitions
    constexpr int kLedPinBlue = 45;
    constexpr int kLedPinGreen =36;
    constexpr int kThermistorPin = 3;
    constexpr int kCurrentAmpEnable = 42;

    constexpr int kExtMicAIN4 = 28;

    constexpr int kTactileSwitchPin =34;

    //With the upgrade to adafruit:nrf52 1.6.0 the existing approach via
    //UI.hpp no longer worked. It is replaced by a call to attachInterrupt
    //from the NRF SDK. 
    constexpr int kTactileSwitchPin_nrf = 7 ;

    
    //PINs used for TTL input, start / stop respectively
    constexpr int kTTL1Pin     = 5;
    constexpr int kTTL1Pin_nrf = 15;

    constexpr int kLedPinExtra     = 4;
    
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
