// SPDX-License-Identifier: AGPL-3.0-or-later

#include <stdint.h>
#include <cmath>
#include <vector>

#ifndef SAMPLECACHE_HPP_
#define SAMPLECACHE_HPP_


/*
 * Cache for the sine wave that is used to generate the stimulation.
 *
 * The main constraint here is to generate the next 8 samples fast
 * enough given the current sampling rate.  From the different methods
 * I've benchmarked, this one turned out to be the fastest (on the ARM
 * nrf52840 architecture). Its fast enough because it avoids lots of
 * arithmetics (and specifically divisions) during the sample
 * computation.
 */

class SampleCache {
public:
    
    explicit SampleCache(
	uint32_t samplerate,
	uint32_t stimfreq) :
	
	// +7 samples of slack in the table to handle worst case call
	// where the table is queried on the last sample of a
	// stimcycle (and still needs to provide 8 samples)	
	samples_needed_(samplerate / stimfreq + 7),
	cache_(samples_needed_)
	{
	    init_cache_(samplerate, stimfreq);
	}


    uint16_t get_sample(uint16_t i, uint16_t volume) const {
	return (uint16_t) (volume + volume * cache_[i]);
    }
    
    
private:
    const unsigned samples_needed_;    
    std::vector<float> cache_;
    
    constexpr static float pi() { return std::atan(1)*4; }
    
    void init_cache_(uint32_t samplerate, uint32_t stimfreq) {				   
	for(uint32_t i = 0; i < samples_needed_; i++) 
	    cache_[i] = std::sin (2 * pi() * i * stimfreq / samplerate);	
    }

    

};

#endif
