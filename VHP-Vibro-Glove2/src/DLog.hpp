// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef DLOG_HPP_
#define DLOG_HPP_

#include <array>
#include <algorithm>

class DLog {
private:
    static constexpr uint16_t size_ = 6000;
    static constexpr uint16_t channels_ = 8;

    typedef std::array<float, size_> sarray;

    std::array<sarray, channels_> logs_;
    std::array<uint16_t, channels_> sample_count_;    
    
public:
    DLog() : logs_({{0.0}}), sample_count_({0}) { }
	

    void reset() {
	sample_count_.fill(0);
    }
    
    void log(const uint32_t channel, const float val) {
	if(channel > channels_) return; // out-of-range

	if(sample_count_[channel] < size_) {
	    logs_[channel][sample_count_[channel]++] = val;
	}
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
