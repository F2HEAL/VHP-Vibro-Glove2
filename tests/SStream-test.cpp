// SPDX-License-Identifier: AGPL-3.0-or-later

#include <iostream>
#include "arduino-mock.hpp"


using namespace std;

#include "VHP-Vibro-Glove2/src/SStream.hpp"

int main() 
{
    const int samplerate = 2000;
    
    
    SStream stream(
		true,    //chan8
		samplerate,   //samplerate
		250,      //stimfreq
		100,      //stimduration
		8000,     //cycleperiod
		5,        //pauzecycleperiod,
		2,        //pauzedcycles
		250,       //jitter
		64,       //volume
		false);    //test_mode

    for(uint16_t sample_frame = 0; sample_frame < 20000; sample_frame++) {
	
	for(uint8_t chan = 0; chan < stream.channels(); chan++) {
	    const uint16_t* samples = stream.chan_samples(chan);
	    
	    cout << 1000 * sample_frame * 8 / samplerate << "ms "
		 << "cycle " << sample_frame << ", chan " << (int) chan << " : ";
	    for(uint8_t s=0; s<8; s++)
	     	cout << samples[s] << "-";
	    cout << endl;
	}

	stream.next_sample_frame();
    }	
    
    return 0;
}

    
