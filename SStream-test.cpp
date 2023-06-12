#include <iostream>
#include "arduino-mock.hpp"


using namespace std;

#include "VHP-Vibro-Glove2/SStream.hpp"

int main() 
{
    SStream stream(
	true,    //chan8
	20000,   //samplerate
	250,     //stimfreq
	100,     //stimduration
	1000,     //cycleperiod
	5,       //pauzecycleperiod,
	2);      //pauzedcycles
	

    for(uint16_t sample_cycle = 0; sample_cycle < 20000; sample_cycle++) {
	
	for(uint8_t chan = 0; chan < 8; chan++) {
	    uint16_t samples[8] = {128, 128, 128, 128,  128, 128, 128, 128 };
	    stream.chan_samples(chan, samples);
	    cout << "cycle " << sample_cycle << ", chan " << (int) chan << " : ";
	    for(uint8_t s=0; s<8; s++)
	     	cout << samples[s] << "-";
	    cout << endl;
	}

	stream.next_sample_frame();
    }	
    
    return 0;
}

    
