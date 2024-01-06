// SPDX-License-Identifier: AGPL-3.0-or-later

#include <iostream>
#include <array>
#include "arduino-mock.hpp"


#include "../VHP-Vibro-Glove2/src/SStream.hpp"
#include "../VHP-Vibro-Glove2/src/Settings.hpp"
#include "../VHP-Vibro-Glove2/src/BoardDefs.hpp"

using namespace audio_tactile;
using namespace std;


uint8_t g_volume = 100;

uint16_t g_volume_lvl = g_volume * g_settings.vol_amplitude / 100;


class MockPwm 
{
    enum {
	kIrqPriority = 7,  // Lowest priority.
	kNumModules = 3,
	kChannelsPerModule = 4,
	kSamplesPerModule = kNumPwmValues * kChannelsPerModule,
    };


public:

    array<uint16_t, kNumModules * kNumPwmValues * kChannelsPerModule> pwm_buffer_;

    // Gets pointer to the start of `channel` in pwm_buffer_.
    uint16_t* GetChannelPointer(int channel) {
	return pwm_buffer_.data() +
		kSamplesPerModule * (channel / kChannelsPerModule) +
		(channel % kChannelsPerModule);
    }

    array<uint16_t, kNumPwmValues> GetChannel(int channel) {
	array<uint16_t, kNumPwmValues> result;

	auto start = GetChannelPointer(channel);

	for(auto i=0; i < kNumPwmValues; i++) {
	    result[i] = start[i*kChannelsPerModule];
	}

	return result;
	
    }

    void SilenceChannel(int channel, uint16_t volume) {
	uint16_t* dest = GetChannelPointer(channel);
	for (int i = 0; i < kNumPwmValues; ++i) {
	    dest[i * kChannelsPerModule] = volume;
	}
    }
    
} PwmTactor;

bool test1() 
{
    SStream ss(
	g_settings.chan8,
	g_settings.samplerate,
	g_settings.stimfreq,
	g_settings.stimduration,
	g_settings.cycleperiod,
	g_settings.pauzecycleperiod,
	g_settings.pauzedcycles,
	g_settings.jitter,
	g_volume_lvl,
	g_settings.test_mode);


    for(auto n=0; n<8000; n++) {

	
	const auto active_channel = ss.current_active_channel();
	
	for(uint32_t channel = 0; channel < ss.channels(); channel++)
	    if(channel==active_channel) {
		uint16_t* cp = PwmTactor.GetChannelPointer(channel);
		ss.set_chan_samples(cp, channel);
	    } else {
		PwmTactor.SilenceChannel(channel, g_volume_lvl);		
	    }

	ss.next_sample_frame();	

	const auto t0 = 1000.0 * n * 8 / g_settings.samplerate;
	
	for(auto sample=0; sample<8; sample++) {
	    const auto t = t0 + sample * 1000.0 / g_settings.samplerate;
	    cout << n << ";" << t << ";";
	    
	    for(uint32_t channel = 0; channel < ss.channels(); channel++) {
		const auto samples_base = PwmTactor.GetChannel(channel);
		cout << samples_base[sample] << ";";
	    }
	    cout << endl;
	}
    }
	
    return true;
}

int main() 
{
    test1();
    
    return 0;
}

    
