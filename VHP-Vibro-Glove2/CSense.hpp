// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef CSENSE_HPP_
#define CSENSE_HPP_

#include <stdint.h>
#include <math.h>

/**
 * CSense - Current sensing
 */

class CSense {
public:

  explicit CSense(
		  uint32_t samplerate,
		  float kAttackTimeConstS,
		  float kDecayTimeConstS
		  ):
    attack_coef_(1.0f - exp(-1.0f / (kAttackTimeConstS * samplerate))),
    decay_coef_(1.0f - exp(-1.0f / (kDecayTimeConstS * samplerate))),
    peak_{0.0}, samples_{0}
  {
    init_counters();
  }
private:
  const float attack_coef_;
  const float decay_coef_;
  
  float peak_[8];
  uint16_t samples_[8];
public:
  /**
   * reset internal state counters
   */
  void init_counters() {
    for(uint16_t i=0; i<8; i++) {
      peak_[i] = 0.0;
      samples_[i] = 0;
    }
  }

  void record(uint32_t channel, float val) {
    samples_[channel]++;
    
    const float peak = peak_[channel];
    const float coef = val >= peak ? attack_coef_ : decay_coef_;
    peak_[channel] += coef * (val - peak);
  }

  float get_peak(uint32_t channel) { return peak_[channel]; }

  float get_samples(uint32_t channel) { return samples_[channel]; }
  
};


#endif
