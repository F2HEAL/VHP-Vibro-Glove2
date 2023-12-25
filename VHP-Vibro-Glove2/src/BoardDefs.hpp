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
