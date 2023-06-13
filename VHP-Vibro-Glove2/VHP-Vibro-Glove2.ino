#include "pwm_sleeve.h"
#include "board_defs.h"


#include "SStream.hpp"

using namespace audio_tactile;

// Output sequence for board Apollo84 hardware
//int order_pairs[8] = {0, 3, 4, 5, 11, 9, 8, 6};

// Output sequence for Godef Hardware
uint16_t order_pairs[8] = {4, 5, 6, 7, 8, 9, 10, 11}; 


SStream *p;

void setup() {
    p = new     SStream(
	true,    //chan8
	46875,   //samplerate 
	250,     //stimfreq
	100*10,     //stimduration
	888*10,     //cycleperiod
	5,       //pauzecycleperiod,
	2,       //pauzedcycles
	64);     //volume


    SleeveTactors.OnSequenceEnd(OnPwmSequenceEnd);
    SleeveTactors.Initialize();

    SleeveTactors.SetUpsamplingFactor(1);

    // Warning: issue only in Arduino. When using StartPlayback() it crashes.
    // Looks like NRF_PWM0 module is automatically triggered, and triggering it
    // again here crashes ISR. Temporary fix is to only use nrf_pwm_task_trigger
    // for NRF_PWM1 and NRF_PWM2. To fix might need a nRF52 driver update.
    nrf_pwm_task_trigger(NRF_PWM1, NRF_PWM_TASK_SEQSTART0);
    nrf_pwm_task_trigger(NRF_PWM2, NRF_PWM_TASK_SEQSTART0);
}


void OnPwmSequenceEnd() {
    p->next_sample_frame();

    for(uint32_t i = 0; i < 8; i++) 
	SleeveTactors.UpdateChannel(order_pairs[i], p->chan_samples(i));
    
}

void loop() {
    // Sleep
    __WFE(); // Enter System ON sleep mode (OFF mode would stop timers)
    __SEV(); // Make sure any pending events are cleared
    __WFE(); // More info about this sequence:
// devzone.nordicsemi.com/f/nordic-q-a/490/how-do-you-put-the-nrf51822-chip-to-sleep/2571    
}
