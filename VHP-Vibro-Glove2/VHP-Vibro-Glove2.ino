// SPDX-License-Identifier: AGPL-3.0-or-later

#include "max14661.h"
#include "pwm_sleeve.h"
#include "board_defs.h"


#include "SStream.hpp"
#include "CSense.hpp"

using namespace audio_tactile;

// Output sequence for board Apollo84 hardware
//int order_pairs[8] = {0, 3, 4, 5, 11, 9, 8, 6};

// Output sequence for Godef Hardware
uint16_t order_pairs[8] = {4, 5, 6, 7, 8, 9, 10, 11}; 


SStream *sstream;
CSense *csense;

const uint32_t volume = 264;
const uint32_t samplerate = 46875;

void setup() {
    nrf_gpio_cfg_output(kLedPinBlue);
    nrf_gpio_cfg_output(kLedPinGreen);
    nrf_gpio_pin_set(kLedPinBlue);
    
    //8 channel mode
    sstream = new     SStream(
	    true,     //chan8
	    samplerate,  //samplerate 
	    250,      //stimfreq
	    100,      //stimduration
	    1332,     //cycleperiod
	    5,        //pauzecycleperiod,
	    2,        //pauzedcycles
	    235,      //jitter
	    volume,       //volume
	    false);   //test_mode

    //4 channel mode.
    /*
    sstream = new     SStream(
	    false,    //chan8
	    46875,    //samplerate 
	    250,      //stimfreq
	    100,      //stimduration
	    666,      //cycleperiod
	    5,        //pauzecycleperiod,
	    2,        //pauzedcycles
	    235,      //jitter
	    64,       //volume
	    false);   //test_mode
    */

    csense = new CSense(samplerate,
			0.0001f, // attack  time const
			1.5f); //decay time const

    nrf_gpio_cfg_output(kCurrentAmpEnable);
    EnableCurrentAmp();
    Multiplexer.Initialize();
    
    SleeveTactors.OnSequenceEnd(OnPwmSequenceEnd);
    SleeveTactors.Initialize();
    
    SleeveTactors.SetUpsamplingFactor(1);

    // Warning: issue only in Arduino. When using StartPlayback() it crashes.
    // Looks like NRF_PWM0 module is automatically triggered, and triggering it
    // again here crashes ISR. Temporary fix is to only use nrf_pwm_task_trigger
    // for NRF_PWM1 and NRF_PWM2. To fix might need a nRF52 driver update.
    nrf_pwm_task_trigger(NRF_PWM1, NRF_PWM_TASK_SEQSTART0);
    nrf_pwm_task_trigger(NRF_PWM2, NRF_PWM_TASK_SEQSTART0);

    nrf_gpio_pin_clear(kLedPinBlue);
}


void OnPwmSequenceEnd() {
    static uint16_t active_channel = 65535;

    
    bool new_cycle_started = sstream->next_sample_frame();
    // print csense data and reinit when new cycle starts
    if(new_cycle_started) {
	if(csense->get_peak(0) > 0) {
	    Serial.println("Channel\tIPeak\tSamples\t");
	    for(uint32_t i = 0; i < sstream->channels(); i++) {
		float peak = csense->get_peak(i);
		uint16_t samples = csense->get_samples(i);
		
		Serial.print(order_pairs[i]-3); Serial.print("\t");
		Serial.print(peak);             Serial.print("\t");
		Serial.println(samples);
	    }
	}
	csense->init_counters();
    }
    
    
    for(uint32_t i = 0; i < sstream->channels(); i++)  {

      // play silence or sample for channel i
      const uint16_t* buf = sstream->chan_samples(i);
      SleeveTactors.UpdateChannel(order_pairs[i], buf);

      
      // check if this channel i is playing something non-silence?
      // this will only be valid for one channel i
      if(buf[0] + buf[1] != 2 * volume) {

	// has the active channel changed?
	if(active_channel != i) {
	  active_channel = i;
	  Multiplexer.ConnectChannel(order_pairs[i]);
	}

	// record the reading for this channel
	const float adc_value = analogRead(A6);
	csense->record(i, adc_value);
      }
    }
}

void loop() {
    // Sleep
    __WFE(); // Enter System ON sleep mode (OFF mode would stop timers)
    __SEV(); // Make sure any pending events are cleared
    __WFE(); // More info about this sequence:
// devzone.nordicsemi.com/f/nordic-q-a/490/how-do-you-put-the-nrf51822-chip-to-sleep/2571    
}

void EnableCurrentAmp() { nrf_gpio_pin_write(kCurrentAmpEnable, 0); }
