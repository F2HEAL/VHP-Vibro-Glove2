#ifndef ARDUINO_MOCK_
#define ARDUINO_MOCK_

#include <stdlib.h>
#include <chrono>

unsigned long micros() 
{
    using namespace std::chrono;
    
    return duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
}


void randomSeed(unsigned long seed) 
{
    srand((unsigned int) seed);
}

unsigned random(unsigned r) 
{
    return rand() % r;
}    



#endif
