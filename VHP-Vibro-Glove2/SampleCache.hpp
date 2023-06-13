#include <stdint.h>
#include <math.h>

#ifndef SAMPLECACHE_HPP_
#define SAMPLECACHE_HPP_

class SampleCache {
public:
    uint16_t* cache_;
    uint16_t silence_[8];
    
    explicit SampleCache(
	uint16_t samplerate,
	uint16_t stimfreq,
	uint16_t volume)
	: cache_(0), silence_{0}
	{
	    init_cache_(samplerate, stimfreq, volume);
	    init_silence_(volume);
	}


    uint16_t sample(uint16_t sample) const { return cache_[sample]; }
	
private:

    void init_cache_(uint16_t samplerate, uint16_t stimfreq, uint16_t volume) {
	// 7 samples of slack in the table to handle worst case call
	// where the table is queried on the last sample of a
	// stimcycle (and still needs to provide 8 samples)
	const uint16_t samples_needed = samplerate / stimfreq + 7; 
								   
	cache_ = new uint16_t[samples_needed];
	
	const float pi2 = 2 * 3.141592;
	for(uint16_t i = 0; i < samples_needed; i++) 
	    cache_[i] = volume
		+ volume * sin (pi2 * i * stimfreq / samplerate);	
    }

    void init_silence_(uint16_t volume) {
	for(uint16_t i=0; i < 8; i++) {
	    silence_[i] = volume;
	}
    }
};

    

#endif
