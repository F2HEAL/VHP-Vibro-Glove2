// Copyright 2020-2021 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//
// Battery monitor: callback if battery voltage is low.
//
// This driver provides a callback if the battery voltage is too low, and it can
// measure the battery voltage. The battery voltage is measured using a voltage
// divider. A low power comparator (LPCOMP) hardware module is used to check
// when the battery voltage is low and trigger an interrupt.
// https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.nrf52832.ps.v1.1%2Flpcomp.html
// To measure the analog battery voltage, SAADC was used.
// Note: the low power comparator can run only for one of the battery monitor or
// temperature monitor at a time

#ifndef BATTERYMONITOR_HPP_
#define BATTERYMONITOR_HPP_

#include <cstdint>

#include "BoardDefs.hpp"
#include "nrf_gpio.h"       // NOLINT(build/include)

namespace {
    extern "C" {
    
	static uint8_t lpcomp_event;

	static void (*battery_callback)(void);

	void on_lpcomp_trigger(void (*function)(void)) { battery_callback = function; }

	uint8_t get_lpcomp_event(void) { return lpcomp_event; }

	void LPCOMP_COMP_IRQHandler(void) {
	    // Clear event.	
	    NRF_LPCOMP->EVENTS_CROSS = 0;

	    // Sample the LPCOMP stores its state in the RESULT register.
	    NRF_LPCOMP->TASKS_SAMPLE = 1;
	    lpcomp_event = NRF_LPCOMP->RESULT;
	
	    battery_callback();
	}
    }
}



namespace audio_tactile {

    // Pin definitions.    
    enum {
	// The low power comparator (LPCOMP) and analog-to-digital converter (ADC) are
	// connected to the same pin, but they are enumerated differently in hardware
	// abstraction layer (HAL), so
	// we need two separate definitions here.
	kLowPowerCompPin = LPCOMP_PSEL_PSEL_AnalogInput6,  // AIN pin 6 (P0.30).
	kLowPowerCompAdcPin = SAADC_CH_PSELP_PSELP_AnalogInput6
    };

    
    class BatteryMonitor {
    public:
	// Configure the battery pin and initialize interrupts.
	// This function starts the listener (interrupt handler) as well.
	void InitializeLowVoltageInterrupt() {
	    // Enable interrupt on LPCOMP CROSS event.
	    NRF_LPCOMP->INTENSET = LPCOMP_INTENSET_CROSS_Msk;

	    // Clear previous and enable interrupts. Set priority.
	    NVIC_DisableIRQ(LPCOMP_IRQn);
	    NVIC_ClearPendingIRQ(LPCOMP_IRQn);
	    NVIC_SetPriority(LPCOMP_IRQn, kLowPowerCompIrqPriority);
	    NVIC_EnableIRQ(LPCOMP_IRQn);

	    // We want to trigger low battery warning around 3.5 V
	    // Going through the voltage divider: Vsense = 0.71 * V_battery
	    // The trigger can only be one of the 16 values.
	    // Theoretically, set input source to 6/8 of Vdd = ((3.0V) * (6/8))/0.71
	    // = 3.55 V_battery. However, the high impedance from  the voltage divider
	    // can load the comparator, so threshold should be measured with a
	    // variable power supply to be sure.
	    NRF_LPCOMP->REFSEL |=
		(LPCOMP_REFSEL_REFSEL_Ref6_8Vdd << LPCOMP_REFSEL_REFSEL_Pos);

	    // Set reference input source to an analog input pin.
	    NRF_LPCOMP->PSEL |= (kLowPowerCompPin << LPCOMP_PSEL_PSEL_Pos);

	    // Enable and start the low power comparator.
	    NRF_LPCOMP->ENABLE = LPCOMP_ENABLE_ENABLE_Enabled;
	    NRF_LPCOMP->TASKS_START = 1;
	}


	// Stop the comparator for the battery monitor.
	void End() {
	    // Disable the low power comparator.
	    NRF_LPCOMP->ENABLE = LPCOMP_ENABLE_ENABLE_Disabled;
	    // Disable the interrupt handler.
	    NVIC_DisableIRQ(LPCOMP_IRQn);
	    NVIC_ClearPendingIRQ(LPCOMP_IRQn);
	}


	// Allows the user to add low battery warning in other parts of firmware.
	void OnLowBatteryEventListener(void (*function)(void)) {
	    on_lpcomp_trigger(function);
	}


	// Returns why the interrupt happened. Two options are:
	// 0 - triggered because under reference voltage.
	// 1 - triggered because over reference voltage.
	uint8_t GetEvent() const { return get_lpcomp_event(); }

	// Use one-shot mode to read the battery voltage.
	int16_t MeasureBatteryVoltage() {
	    // TODO Make sure it works nicely with analog microphone sharing the
	    // ADC. This code was inspired by the Arduino AnalogRead library.
	    // https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/master/cores/nRF5/wiring_analog_nRF52.c
	    static int16_t value = 0;
	    NRF_SAADC->RESOLUTION = SAADC_RESOLUTION_VAL_12bit;
	    NRF_SAADC->ENABLE = (SAADC_ENABLE_ENABLE_Enabled << SAADC_ENABLE_ENABLE_Pos);

	    // Use ADC channel 0.
	    // Set the acquisition time to 40 us (max), since the voltage divider has high
	    // impedance. Select internal reference (0.6V), which gives us 3.6 V range.
	    NRF_SAADC->CH[0].CONFIG =
		((SAADC_CH_CONFIG_RESP_Bypass << SAADC_CH_CONFIG_RESP_Pos) &
		 SAADC_CH_CONFIG_RESP_Msk) |
		((SAADC_CH_CONFIG_RESN_Bypass << SAADC_CH_CONFIG_RESN_Pos) &
		 SAADC_CH_CONFIG_RESN_Msk) |
		((SAADC_CH_CONFIG_GAIN_Gain1_6 << SAADC_CH_CONFIG_GAIN_Pos) &
		 SAADC_CH_CONFIG_GAIN_Msk) |
		((SAADC_CH_CONFIG_REFSEL_Internal << SAADC_CH_CONFIG_REFSEL_Pos) &
		 SAADC_CH_CONFIG_REFSEL_Msk) |
		((SAADC_CH_CONFIG_TACQ_40us << SAADC_CH_CONFIG_TACQ_Pos) &
		 SAADC_CH_CONFIG_TACQ_Msk) |
		((SAADC_CH_CONFIG_MODE_SE << SAADC_CH_CONFIG_MODE_Pos) &
		 SAADC_CH_CONFIG_MODE_Msk) |
		((SAADC_CH_CONFIG_BURST_Disabled << SAADC_CH_CONFIG_BURST_Pos) &
		 SAADC_CH_CONFIG_BURST_Msk);

	    // Set the pin to analog input.
	    // PSELN has no effect for single ended mode.
	    NRF_SAADC->CH[0].PSELP = kLowPowerCompAdcPin;

	    // Pointer to the ADC buffer for Easy DMA.
	    NRF_SAADC->RESULT.PTR = (uint32_t)&value;
	    NRF_SAADC->RESULT.MAXCNT = 1;  // One sample.

	    // Trigger immediate  ADC sampling.
	    NRF_SAADC->TASKS_START = 0x01UL;

	    while (!NRF_SAADC->EVENTS_STARTED) {
	    }

	    NRF_SAADC->EVENTS_STARTED = 0x00UL;
	    NRF_SAADC->TASKS_SAMPLE = 0x01UL;

	    while (!NRF_SAADC->EVENTS_END) {
	    }

	    NRF_SAADC->EVENTS_END = 0x00UL;
	    NRF_SAADC->TASKS_STOP = 0x01UL;

	    while (!NRF_SAADC->EVENTS_STOPPED) {
	    }

	    NRF_SAADC->EVENTS_STOPPED = 0x00UL;

	    // Sometimes ADC values can be negative, just make them 0.
	    if (value < 0) {
		value = 0;
	    }

	    // Disable the ADC.
	    NRF_SAADC->ENABLE = (SAADC_ENABLE_ENABLE_Disabled << SAADC_ENABLE_ENABLE_Pos);

	    return value;
	}


	// This function converts the raw ADC reading into actual battery voltage.
	// The battery voltage can be converted later into battery percentage. This
	// could be done by measuring discharge curve.
	float ConvertBatteryVoltageToFloat(int16_t raw_adc_battery_reading) {
	    float converted_v;

	    // Voltage divider has 806 k and 2M Ohm resistors.
	    // The following is the scale factor derived from voltage divider equation.
	    // https://en.wikipedia.org/wiki/Voltage_divider
	    // V_out = kVoltageDivider * V_in.
	    // With gcc optimization level 1 (O1) and above const calculations are
	    // performed in compile time.
	    const float kVoltageDivider = (2000.0f) / (806.0f + 2000.0f);

	    // The ADC gain is set to 1/6.
	    const float kGainADC = 1.0f / 6.0f;

	    // This is the internal reference voltage used by ADC.
	    const float kInternalRefVoltage = 0.6f;

	    // The resolution of the ADC is 12 bits (2^12).
	    const float kBitsResolution = 4096.0f;

	    // The input range is reference voltage divided by gain, according to the
	    // SAADC datasheet:
	    // https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.nrf52832.ps.v1.1%2Fsaadc.html
	    const float kInputRange = (kInternalRefVoltage) / (kGainADC);

	    // Bit to volts conversion factor.
	    const float kBitsToVolts = kInputRange / kBitsResolution;

	    converted_v = (kBitsToVolts / kVoltageDivider) * raw_adc_battery_reading;

	    return converted_v;
	}


    private:
	// Low power comparator definitions.
	enum {
	    kLowPowerCompIrqPriority = 7  // lowest priority
	};


    };


    BatteryMonitor PuckBatteryMonitor;

}  // namespace audio_tactile

#endif  // BATTERYMONITOR_HPP_
