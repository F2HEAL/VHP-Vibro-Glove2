#include <stdint.h>

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
 * PWM driver, sample_cycles are used. In a sample_frame, 8 samples
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
     * @param volume - value for volume
     * @param jitter - TODO
     */
    explicit SStream(
	bool chan8,
	uint32_t samplerate,
	uint32_t stimfreq,
	uint32_t stimduration,
	uint32_t cycleperiod,
	uint32_t pauzecycleperiod,
	uint32_t pauzedcycles,
	uint32_t volume
	) : frame_counter_(0), cycle_counter_(0), channel_order_{0}, 
	    chan8_(chan8), samplerate_(samplerate), stimfreq_(stimfreq), stimduration_(stimduration),
	    cycleperiod_(cycleperiod), pauzecycleperiod_(pauzecycleperiod), pauzedcycles_(pauzedcycles),
	    samples_per_frame_(8),
	    sample_cache_(samplerate, stimfreq, volume)
	{
	    for(uint32_t i=0; i < 8; i++) 
		channel_order_[i] = i;
	}
private:
    // internal state
    uint32_t frame_counter_;
    uint32_t cycle_counter_;
    uint32_t channel_order_[8];
    
    // set by constructor
    const bool chan8_;
    const uint32_t samplerate_;
    const uint32_t stimfreq_;
    const uint32_t stimduration_;
    const uint32_t cycleperiod_;
    const uint32_t pauzecycleperiod_;
    const uint32_t pauzedcycles_;
    const uint32_t samples_per_frame_;

    const SampleCache sample_cache_;

private:
    //private functions
    /**
     * @return Total number of unique active channels
     */
    uint32_t channels_() const { return chan8_ ? 8 : 4; }

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
    uint32_t active_channel_number_(uint32_t sample) const { return sample / ( samples_per_cycle_() / channels_()); }

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
	    && sample % (samples_per_cycle_() / channels_())
	               < stimduration_ * samplerate_ / 1000; }
    
public:
    /**
     * Advances internal state to the next sample frame
     */
    void next_sample_frame() {
	frame_counter_++;

	if(frame_counter_  > samples_per_cycle_() / samples_per_frame_) {
	    frame_counter_ = 0;
	    
	    shuffle_channel_order();

	    cycle_counter_++;
	    if(cycle_counter_ >= pauzecycleperiod_)
		cycle_counter_ = 0;
	    
	}
    }

    /**
     * chan_samples() - produces the value for samples_per_frame_
     * samples in the designated buffer. 
     *
     * @param chan - queried channel
     * @return pointer to array holding samples_per_frame_ values
     */

    
    const uint16_t* chan_samples(uint32_t chan) {
	const uint32_t frame_first_sample = frame_counter_ * samples_per_frame_;
	if(cycle_is_pauzed_() || !channel_is_playing_(frame_first_sample, chan)) 
	    return sample_cache_.silence_;
	else
	    return sample_cache_.cache_ + frame_first_sample % samples_per_stimperiod_();
    }

private:
    
    /**    
     * Shuffles the values in channel_order_ in such a way that the value
     * of the last element differs from the first (to avoid activating the
     * same channel consecutively)
     */    
    void shuffle_channel_order() {
	randomSeed(micros());
    
	int rj = random(channels_() - 1);
	uint32_t tmp = channel_order_[rj];
	channel_order_[rj] = channel_order_[0];
	channel_order_[0] = tmp;
    
	for(uint32_t i=1; i<channels_() - 1; i++) {
	    rj = random(channels_() - i);
	    uint32_t tmp = channel_order_[i];

	    channel_order_[i] = channel_order_[i+rj];
	    channel_order_[i+rj] = tmp;
	}
    }
};

#endif

