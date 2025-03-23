// SPDX-License-Identifier: AGPL-3.0-or-later

#include <stdint.h>
#include <array>
#include <numeric>
#include <algorithm>

#include "SampleCache.hpp"

#ifndef SSTREAM_HPP_
#define SSTREAM_HPP_

/**
 * SStream - class to generate vibration stream on 4 or 8 channels
 *
 * The exact generated pattern is determined by the parameters to the
 * constructor
 *
 * As this class is contructed for usage with Adafruit Feather nRF52
 * PWM driver, sample_frames are used. In a sample_frame, 8 samples
 * have been consumed by the hardware.
 *
 * next_sample_frame() indicates that a cycle has passed
 * chan_samples() produces the 8 samples for the given channel in the
 * current cycle
 */

class SStream {
public:    
    
    /**
     * SStream - Create new SStream object
     *
     * @param chan8 - True selects 8 independent channels, False
     *         results in 2 x 4 channels mirrored
     * @param samplerate - samplerate of PWM driver
     * @param stimfreq - Frequency of finger stimulation in Hz
     * @param stimduration - Duration of the finger stimulation in ms
     * @param cycleperiod - Duration of one cycle (stimulation of all fingers)
     * @param pauzecycleperiod - Duration (in cycles) of one
     *         pauze-cycle
     * @param pauzedcycles - How many cycles within one
     *         pauzecycleperiod are to be silicend
     * @param jitter - Jitter in promile of cycleperiod / channels
     *        (0...1000), see calc_channel_jitter_()
     * @param volume - value for volume, range for valid value is 0..512
     * @param test_mode - don't randomize channel order
     * @param single_channel - when non-zero, only trigger specified channel
     */
    explicit SStream(
	bool chan8,
	uint32_t samplerate,
	uint32_t stimfreq,
	uint32_t stimduration,
	uint32_t cycleperiod,
	uint32_t pauzecycleperiod,
	uint32_t pauzedcycles,
	uint16_t jitter,
	uint32_t volume,
	bool test_mode = true,
	uint16_t single_channel = 0
	) : frame_counter_(0), cycle_counter_(0), channel_order_{0}, channel_jitter_{0},
	    chan8_(chan8), samplerate_(samplerate), stimfreq_(stimfreq), stimduration_(stimduration),
	    cycleperiod_(cycleperiod), pauzecycleperiod_(pauzecycleperiod), pauzedcycles_(pauzedcycles),
	    max_jitter_(jitter * cycleperiod_ / channels() / 1000),
	    volume_(volume),
	    test_mode_(test_mode),
	    sample_cache_(samplerate, stimfreq)
	{
	    randomSeed(micros());

	    if(test_mode && single_channel > 0 && single_channel <= max_channels) 
		std::fill(channel_order_.begin(), channel_order_.end(), single_channel - 1);
	    else
		std::iota(channel_order_.begin(), channel_order_.end(), 0);
	    

	    if(!test_mode_)
		shuffle_channel_order_();
	    
	    if(max_jitter_ > 0)
		calc_channel_jitter_();
	}
private:
    constexpr static size_t max_channels = 8;
    
    // internal state
    uint32_t frame_counter_;
    uint32_t cycle_counter_;
    //uint32_t channel_order_[8];
    std::array<uint32_t, max_channels> channel_order_;
    
    //int32_t channel_jitter_[8];
    std::array<int32_t, max_channels> channel_jitter_;
    
    
    // set by constructor
    const bool chan8_;
    const uint32_t samplerate_;
    const uint32_t stimfreq_;
    const uint32_t stimduration_;
    const uint32_t cycleperiod_;
    const uint32_t pauzecycleperiod_;
    const uint32_t pauzedcycles_;
    const uint32_t max_jitter_;
    const uint32_t volume_;
    constexpr static uint32_t samples_per_frame_ = 8;
    const bool test_mode_;
    
    const SampleCache sample_cache_;

private:
    /**
     * @return Number of samples in a single cycle
     */
    uint32_t samples_per_cycle_() const { return samplerate_ * cycleperiod_ / 1000; }

    uint32_t samples_per_stimperiod_() const { return samplerate_ / stimfreq_; }
    
    /**
     * @return true if the current cycle is pauzed
     */
    bool cycle_is_pauzed_() const { return cycle_counter_ >= pauzecycleperiod_ - pauzedcycles_; }

    /**
     * @input sample - queried sample number
     * @return active channel number for queried sample, thus 0 1 2 ... channels within cycle
     */
    uint32_t active_channel_number_(uint32_t sample) const { return sample / ( samples_per_cycle_() / channels()); }

    /**
     * @input  sample - queried sample number
     * @return active channel for queried sample, thus randomized channels within cycle
     */
    uint32_t active_channel_(uint32_t sample) const { return channel_order_[active_channel_number_(sample)]; }

    /**
     * @input sample - queried sample number
     * @input channel - queried channel number
     * @returns true if the queried channel is active for the queried
     * sample. Note that an active channel can produce silence when
     * stimduration has already passed.
     */
    bool channel_is_active_(uint32_t sample, uint32_t channel) const { return channel == active_channel_(sample); }


    /**
     * @input sample - queried sample number
     * @input channel - queried channel number
     * @returns true if the queried channel is playing for the queried
     * sample. Thus true if stimduration is still ongoing.
     */
    bool channel_is_playing_(uint32_t sample, uint32_t channel) const {
	return channel_is_active_(sample, channel)
	    && sample % (samples_per_cycle_() / channels())
	                < stimduration_ * samplerate_ / 1000; }
    
public:
    /**
     * @return Total number of unique active channels
     */
    uint32_t channels() const { return chan8_ ? 8 : 4; }


    uint32_t current_active_channel() const {
	if(cycle_is_pauzed_())
	    return UINT32_MAX;

	return active_channel_(frame_counter_ * samples_per_frame_);
    }
	
    
    /**
     * Advances internal state to the next sample frame
     */
    void next_sample_frame() {
	frame_counter_++;

	if(frame_counter_  >= samples_per_cycle_() / samples_per_frame_) {
	    frame_counter_ = 0;

	    cycle_counter_++;
	    if(cycle_counter_ >= pauzecycleperiod_)
		cycle_counter_ = 0;

	    if(!cycle_is_pauzed_()) {	
		if(!test_mode_)
		    shuffle_channel_order_();

		if(max_jitter_ > 0)
		    calc_channel_jitter_();
	    }
	}
    }

    /**
     * chan_samples() - produces the value for samples_per_frame_
     * samples in the designated buffer. 
     *
     * @param chan - queried channel
     * @return pointer to array holding samples_per_frame_ values
     */


    enum {
	kChannelsPerModule = 4
    };
    
    void set_chan_samples(uint16_t* dest, uint32_t chan) const {
	const int32_t first_sample = (int32_t) ((frame_counter_ * samples_per_frame_)
						% (samples_per_cycle_() / channels()))  - channel_jitter_[chan];

	if(first_sample < 0 ||
	   first_sample > (int32_t) (stimduration_ * samplerate_ / 1000))
	    set_silence_(dest);
	else {
	    auto base = first_sample % (samplerate_ / stimfreq_);
	    for(unsigned i=0; i < samples_per_frame_; i++)
		dest[i*kChannelsPerModule]= sample_cache_.get_sample(base + i, volume_);
	}
    }
private:

    void set_silence_(uint16_t* dest) const {
	for(unsigned i=0; i < samples_per_frame_; i++)
	    dest[i*kChannelsPerModule]=volume_; // play silence
    }

    
    /**    		
     *  Randomize channel order
     */    
    void shuffle_channel_order_() {

	struct RandInt {
	    using result_type = uint32_t;

	    static constexpr result_type min()  {
		return 0;
	    }
	    
	    static constexpr result_type max() {
		return UINT32_MAX-1;
	    }
	    
	    result_type operator()() {
		return random(UINT32_MAX);
	    }
	};
	
	
	std::shuffle(channel_order_.begin(), channel_order_.end(), RandInt());
    }


    /**
     * recalculate Jitter value for each channel
     *
     * In the protocol J is % of 1/8th of cycleperiod so that every
     * start is delayed over ] s0 - J * cycleperiod / 8 , s0 + J
     * cycleperiod / 8 [ (from a uniform distribution)
     *
     * This is equivalent to delaying each input with
     * random(cycleperiod/4) * Jitter / 1000
     *
     * As we support 8 channels, we make this
     * random(cycleperiod / channels)
     */
    
    void calc_channel_jitter_() {
	std::generate(channel_jitter_.begin(), channel_jitter_.end(), [this]() { return random(max_jitter_); });
    }
	
};

#endif

