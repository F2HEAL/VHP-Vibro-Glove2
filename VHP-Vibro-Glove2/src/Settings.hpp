// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef SETTINGS_HPP_
#define SETTINGS_HPP_

#include <stdint.h>

constexpr char name[] = "F2Heal VHP";
constexpr const char* default_parameter_settings = "V25 F250 D100 Y1332 P5 Q2 J235 M0 C0";
                                                    
/**
 * Default settings.
 *
 * These settings will be used when the device is powered up. 
 */
struct Settings {

    /*
     * Configuration constants
     */

 
    const uint32_t default_channels = 8;  /* Number of channels silence is played
					     on when stream is not playing */
  
    const bool     start_stream_on_power_on = false; /* Start Stream on Power On */


    /*
     * The default values for starting the Stream, see Stream.hpp for explanation.
     *
     * These values are configurable using the Bluetooth Web UI.
     */
    bool chan8 = true;
    uint32_t samplerate = 30000;
    uint32_t stimfreq = 40;
    uint32_t stimduration = 8000;
    uint32_t cycleperiod = 64000;
    uint32_t pauzecycleperiod = 1;
    uint32_t pauzedcycles = 0;
    uint16_t jitter = 0;
    uint32_t vol_amplitude = 208;
    bool test_mode = true;
    uint16_t single_channel = 1;
  
} g_settings;


#endif

