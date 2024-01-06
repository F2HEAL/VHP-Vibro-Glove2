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
// Arduino-compatible PWM playback library for the sleeve using the HAL layer.
//
// This library could be used for Linear Resonant Actuators (LRA) or voice
// coils. 
//
// This filtered signal is passed as single-ended input to MAX98306 audio
// amplifiers. The class-D amplifier output is connected to the tactors.
// Amplifier datasheet here:
// https://www.maximintegrated.com/en/products/analog/audio/MAX98306.html
//
// The sleeve uses 6 audio amplifiers and a total of 12 PWM channels.
// There are 3 PWM modules. Each module has 4 channels.
// The values in each channel can be set independently.
// The PWM uses: 192B of RAM with 8 PWM values for each channel (3 module * 4
// channels * 8 values * 2 byte each value)
//
// An example snippet for initialization steps are are as following:
// SleeveTactors.Initialize();
// SleeveTactors.SetNumberRepeats(8);
// SleeveTactors.OnSequenceEnd(on_PWM_sequence_end);
// SleeveTactors.StartPlayback();
//
// After initialization, this code  plays individual tactors one after another.
// for(int pwm_module = 0; pwm_module<3; ++pwm_module) {
//   for (int pwm_channel = 0; pwm_channel<4; ++pwm_channel) {
//     SleeveTactors.UpdatePwmModuleChannel(sin_wave_downsample,
//                                          pwm_module,pwm_channel);
//     vTaskDelay(500);  // a delay of 500 ms.
//     SleeveTactors.SilencePwmModuleChannel(pwm_module,pwm_channel);
//   }
// }
//
// The PWM hardware and registers are described here:
// https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.nrf52832.ps.v1.1%2Fpwm.html
// HAL is described here:
// https://infocenter.nordicsemi.com/topic/com.nordic.infocenter.sdk5.v12.0.0/group__nrf__pwm__hal.html

#ifndef PWMTACTOR_HPP_
#define PWMTACTOR_HPP_

#include <stdint.h>

#include "BoardDefs.hpp"
#include "nrf_pwm.h"

namespace {
    extern "C" {
	static uint8_t pwm_event;

	static void (*pwm_callback)(void);

	void on_pwm_sequence_end(void (*function)(void)) { pwm_callback = function; }

	uint8_t get_pwm_event() { return pwm_event; }
	
	void pwm_irq_handler(NRF_PWM_Type* pwm_module, uint8_t which_pwm_module) {
	    /* Triggered when pwm data is trasfered to RAM with Easy DMA. */
	    if (nrf_pwm_event_check(pwm_module, NRF_PWM_EVENT_SEQSTARTED0)) {
		nrf_pwm_event_clear(pwm_module, NRF_PWM_EVENT_SEQSTARTED0);
	    }
	    /* Triggered after sequence is finished. */
	    if (nrf_pwm_event_check(pwm_module, NRF_PWM_EVENT_SEQEND0)) {
		nrf_pwm_event_clear(pwm_module, NRF_PWM_EVENT_SEQEND0);
		pwm_event = which_pwm_module;
		/* Do pwm_callback before starting the next sequence, so we can modify the
		 * buffer before it gets played.
		 */
		pwm_callback();
		nrf_pwm_task_trigger(pwm_module, NRF_PWM_TASK_SEQSTART0);
	    }
	    /* Triggered when playback is stopped. */
	    if (nrf_pwm_event_check(pwm_module, NRF_PWM_EVENT_STOPPED)) {
		nrf_pwm_event_clear(pwm_module, NRF_PWM_EVENT_STOPPED);
	    }
	}

	void PWM0_IRQHandler() { pwm_irq_handler(NRF_PWM0, 0); }
	void PWM1_IRQHandler() { pwm_irq_handler(NRF_PWM1, 1); }
	void PWM2_IRQHandler() { pwm_irq_handler(NRF_PWM2, 2); }

    }
}

namespace audio_tactile {
    class Pwm {
    public:
	enum {
	    kTopValue = 512,   // Individual PWM values can't be above this number.
	    kUpsamplingFactor = 0,
	};

	// Pins on port 1 are always offset by 32. For example pin 7 (P1.07) is 39.
	enum {
	    kL1Pin = 13,  // On P0.13.
	    kR1Pin = 14,  // On P0.14.
	    kL2Pin = 46,  // On P1.14.
	    kR2Pin = 12,  // On P0.12.
	    kL3Pin = 47,  // On P1.15.
	    kR3Pin = 32,  // On P1.00.
	    kL4Pin = 40,  // On P1.08.
	    kR4Pin = 41,  // On P1.09.
	    kL5Pin = 11,  // On P0.11.
	    kR5Pin = 16,  // On P0.16.
	    kL6Pin = 27,  // On P0.27.
	    kR6Pin = 15,  // On P0.15.

	    kAmpEnablePin1 = 33,  // On 1.01.
	    kAmpEnablePin2 = 44,  // On 1.12.
	    kAmpEnablePin3 = 8,   // On 0.08.
	    kAmpEnablePin4 = 39,  // On 1.07.
	    kAmpEnablePin5 = 43,  // On 1.11.
	    kAmpEnablePin6 = 35   // On 1.03.
	};

	// This function starts the tactors on the sleeve. Also, initializes amplifier
	// pins.
	void Initialize() {
	    // Configure amplifiers shutdowns pin.
	    nrf_gpio_cfg_output(kAmpEnablePin1);
	    nrf_gpio_cfg_output(kAmpEnablePin2);
	    nrf_gpio_cfg_output(kAmpEnablePin3);
	    nrf_gpio_cfg_output(kAmpEnablePin4);
	    nrf_gpio_cfg_output(kAmpEnablePin5);
	    nrf_gpio_cfg_output(kAmpEnablePin6);

	    // Turn on the speaker amplifiers.
	    EnableAmplifiers();

	    // Configure the pins.
	    uint32_t pins_pwm0[4] = {kL1Pin, kR1Pin, kL2Pin, kR2Pin};
	    uint32_t pins_pwm1[4] = {kL3Pin, kR3Pin, kL4Pin, kR4Pin};
	    uint32_t pins_pwm2[4] = {kL5Pin, kR5Pin, kL6Pin, kR6Pin};

	    InitializePwmModule(NRF_PWM0, pins_pwm0);
	    InitializePwmModule(NRF_PWM1, pins_pwm1);
	    InitializePwmModule(NRF_PWM2, pins_pwm2);


	    // Set the buffer pointers. Need to set it before running PWM.
	    // Tricky part here is that the buffer is always represents 4 channels:
	    // <pin 1 PWM> <pin 2 PWM> <pin 3 PWM> <pin 4 PWM> ... <pin 1 PWM>
	    // Even if we only use two pins (as here), we still need to set values for
	    // 4 channels, as easy DMA reads them consecutively.
	    nrf_pwm_seq_cnt_set(NRF_PWM0, 0, kSamplesPerModule);
	    nrf_pwm_seq_ptr_set(NRF_PWM0, 0, pwm_buffer_);
	    nrf_pwm_seq_cnt_set(NRF_PWM1, 0, kSamplesPerModule);
	    nrf_pwm_seq_ptr_set(NRF_PWM1, 0, pwm_buffer_ + kSamplesPerModule);
	    nrf_pwm_seq_cnt_set(NRF_PWM2, 0, kSamplesPerModule);
	    nrf_pwm_seq_ptr_set(NRF_PWM2, 0, pwm_buffer_ + 2 * kSamplesPerModule);


	    // Enable global interrupts for PWM.
	    NVIC_SetPriority(PWM0_IRQn, kIrqPriority);
	    NVIC_EnableIRQ(PWM0_IRQn);
	    NVIC_SetPriority(PWM1_IRQn, kIrqPriority);
	    NVIC_EnableIRQ(PWM1_IRQn);
	    NVIC_SetPriority(PWM2_IRQn, kIrqPriority);
	    NVIC_EnableIRQ(PWM2_IRQn);
	}

	// In the following, the `channel` arg is a zero-based flat 1D index between 0
	// and 11. At a lower level, channels are driven by PWMs modules, each module
	// driving up to 4 channels. Another variation is that audio channels are
	// conventionally indexed starting from 1. So elsewhere, these channels might
	// be referred to by (module, module-channel) or by 1-based index:
	//
	//   `channel`  (module, module-channel)   1-based index
	//   ---------------------------------------------------
	//          0                     (0, 0)               1
	//          1                     (0, 1)               2
	//          2                     (0, 2)               3
	//          3                     (0, 3)               4
	//          4                     (1, 0)               5
	//          5                     (1, 1)               6
	//          6                     (1, 2)               7
	//          7                     (1, 3)               8
	//          8                     (2, 0)               9
	//          9                     (2, 1)              10
	//         10                     (2, 2)              11
	//         11                     (2, 3)              12

	// Sets values of specific channel to volume, so there is nothing to play.
	void SilenceChannel(int channel, uint16_t volume) {
	    uint16_t* dest = GetChannelPointer(channel);
	    for (int i = 0; i < kNumPwmValues; ++i) {
		dest[i * kChannelsPerModule] = volume;
	    }
	}


	// This function is called when sequence is finished.
	void OnSequenceEnd(void (*function)(void)) {
	    on_pwm_sequence_end(function);
	}


	// Gets pointer to the start of `channel` in pwm_buffer_.
	uint16_t* GetChannelPointer(int orig_channel) {
	    const auto channel = order_pairs[orig_channel];
	    
	    return pwm_buffer_ +
		kSamplesPerModule * (channel / kChannelsPerModule) +
		(channel % kChannelsPerModule);
	}

	
    private:
	enum {
	    kIrqPriority = 7,  // Lowest priority.
	    kNumModules = 3,
	    kChannelsPerModule = 4,
	    kSamplesPerModule = kNumPwmValues * kChannelsPerModule,
	};

	// Internal initialization helper.
	void InitializePwmModule(NRF_PWM_Type* pwm_module, uint32_t pins[4]) {
	    // Enable the PWM.
	    nrf_pwm_enable(pwm_module);

	    // Configure the pins.
	    nrf_pwm_pins_set(pwm_module, pins);

	    // `kTopValue` is half the number of clock ticks per PWM sample. The PWM
	    // sample value should be in [0, kTopValue], and is the clock tick to flip
	    // output between high and low (we use NRF_PWM_MODE_UP counter mode). The PWM
	    // sample rate is 16 MHz / kTopValue.
	    //
	    // E.g. with kTopValue = 1024, the sample rate is 15625 Hz.
	    nrf_pwm_configure(pwm_module, NRF_PWM_CLK_8MHz, NRF_PWM_MODE_UP, kTopValue);

	    // Refresh is 1 by default, which means that each PWM pulse is repeated twice.
	    // Set it to zero to avoid repeats. Also can be set whatever with kNumRepeats.
	    nrf_pwm_seq_refresh_set(pwm_module, 0, kUpsamplingFactor);

	    // Set the decoder. Decoder determines how PWM values are loaded into RAM.
	    // We set it to individual, meaning that each value represents a separate pin.
	    nrf_pwm_decoder_set(pwm_module, NRF_PWM_LOAD_INDIVIDUAL, NRF_PWM_STEP_AUTO);

	    // Enable interrupts.
	    nrf_pwm_int_enable(pwm_module, NRF_PWM_INT_SEQSTARTED0_MASK);
	    nrf_pwm_int_enable(pwm_module, NRF_PWM_INT_SEQEND0_MASK);
	}

	// Enable all audio amplifiers with a hardware pin.
	void EnableAmplifiers() {
	    nrf_gpio_pin_write(kAmpEnablePin1, 1);
	    nrf_gpio_pin_write(kAmpEnablePin2, 1);
	    nrf_gpio_pin_write(kAmpEnablePin3, 1);
	    nrf_gpio_pin_write(kAmpEnablePin4, 1);
	    nrf_gpio_pin_write(kAmpEnablePin5, 1);
	    nrf_gpio_pin_write(kAmpEnablePin6, 1);
	}


	// Playback buffer.
	// In "individual" decoder mode, buffer represents 4 channels:
	// <pin 1 PWM 1>, <pin 2 PWM 1>, <pin 3 PWM 1 >, <pin 4 PWM 1>,
	// <pin 1 PWM 2>, <pin 2 PWM 2>, ....
	// Even if we only use two pins, we still need to set values for
	// 4 channels, as easy DMA reads them consecutively.
	// The playback on pin 1 will be <pin 1 PWM 1>, <pin 1 PWM 2>.
	uint16_t pwm_buffer_[kNumModules * kNumPwmValues * kChannelsPerModule];
    };

    Pwm PwmTactor;

}  // namespace audio_tactile

#endif  // PWMTACTOR_HPP_
