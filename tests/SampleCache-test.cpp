#include <iostream>
#include <stdint.h>

using namespace std;

#include "VHP-Vibro-Glove2/SampleCache.hpp"

int main() 
{
    const int samplerate = 15625;
    const int stimfreq = 250;
    const int volume = 128;

    const SampleCache s(samplerate, stimfreq, volume);


    const uint16_t total_samples = samplerate / stimfreq;

    for(unsigned i = 0; i < total_samples; i++) {
	const uint16_t* frame = s.cache_ + i;

	cout << "I: " << i << " samples : ";
	for(unsigned j = 0; j < 8; j++)
	    cout << frame[j] << "-";
	cout << endl;
    }

    const uint16_t* sil = s.silence_;
    cout << "Silence samples : ";
    for(unsigned j = 0; j < 8; j++)
	cout << sil[j] << "-";
    cout << endl;
    
    
    return 0;
}
