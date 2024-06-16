// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef DLOG_HPP_
#define DLOG_HPP_

#include <array>
#include <algorithm>
#include <numeric>
#include <cmath>

class DLog {
private:
    static constexpr uint16_t size_ = 6000;
    static constexpr uint16_t channels_ = 8;

    typedef std::array<float, size_> sarray;

    std::array<sarray, channels_> logs_;
    std::array<uint16_t, channels_> sample_count_;    
    std::array<float, channels_> average_;
    std::array<float, channels_> power_;
    float power_scale_;
public:
    DLog() : logs_({{0.0}}),
	     sample_count_({0}),
	     average_({0.0}),
	     power_({0.0}),
	     power_scale_(0.0)
	{ }
	

    void reset() {
	sample_count_.fill(0);
    }
    
    void log(const uint32_t channel, const float val) {
	if(channel > channels_) return; // out-of-range

	if(sample_count_[channel] < size_) {
	    logs_[channel][sample_count_[channel]++] = val;
	}
    }

    void calc_average() {
	for(int channel = 0; channel < channels_; channel++) {
	    uint32_t sum = std::accumulate(logs_[channel].begin(), logs_[channel].end(), 0);

	    average_[channel] = (float)sum / sample_count_[channel];
	}
    }

    void calc_power() {

	for(int i = 0; i < channels_; i++) 
	    power_[i] = std::accumulate(logs_[i].begin(), logs_[i].end(), 0.0,
					      [&](const float a, const float b) { return a+std::pow((b-average_[i]), 2); });


	//if(power_scale_ == 0.0) {
	power_scale_ = *std::max_element(power_.begin(), power_.end());
	//}
    }
    
    void power_print() {
	calc_average();

	// Serial.print("Average: ");
	// for(int i = 0; i < channels_; i++) {
	//     Serial.print(average_[i]);
	//     Serial.print(" ");
	// }
	// Serial.println();

	Serial.print("Samples: ");
	for(int i = 0; i < channels_; i++) {
	    Serial.print(sample_count_[i]);
	    Serial.print(" ");
	}
	Serial.println();

	
	calc_power();
	
	Serial.print("Power: ");
	for(int i = 0; i < channels_; i++) {
	    Serial.print(power_[i] / power_scale_);
	    Serial.print(" ");
	}
	Serial.println();

    }
    
    
    void serial_print() const {
	const auto max_samples = std::max_element(sample_count_.begin(), sample_count_.end());

	Serial.print("Samples:;");
	Serial.print(*max_samples);
	Serial.println(";");
	for(int i = 0; i < *max_samples; i++) {
	    Serial.print(i);  Serial.print(";");
	    for(int channel = 0; channel < channels_; channel++) {
		if(i<sample_count_[channel]) {    
		    Serial.print(logs_[channel][i]);
		} else {
		    Serial.print(0.0);
		}
		
		Serial.print(";");
	    }
	    Serial.println("");
	}
    }
    
} g_dlog;

    
#endif
