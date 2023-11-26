// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef DLOG_HPP_
#define DLOG_HPP_

class DLog {
private:
    static const uint16_t size_ = 6000;
    float log_[8][size_]{};
    int samples_[8]{};
    
public:
    DLog()  {}

    void reset() {
	for (int i = 0; i < 8; i++) {
	    samples_[i] = 0;
	}
    }
    
    void log(const uint32_t channel, const float val) {
	if(samples_[channel] < size_) {
	    log_[channel][samples_[channel]++] = val;
	}
    }

    float samples(const uint32_t channel) const {
	return samples_[channel];
    }

    void serial_print() const {
	int max_samples = samples_[0];
	for(int i = 1; i < 8; i++) {
	    if(samples_[i] > max_samples) {
		max_samples = samples_[i];
	    }
	}
	
	Serial.print("Samples:;");
	Serial.print(max_samples);
	Serial.println(";");
	for(int i = 0; i < max_samples; i++) {
	    Serial.print(i);  Serial.print(";");
	    for(int channel = 0; channel < 8; channel++) {
		if(i<samples_[channel]) {    
		    Serial.print(log_[channel][i]);
		}
		Serial.print(";");
	    }
	    Serial.println("");
	}
    }
    
} g_dlog;

    
#endif
