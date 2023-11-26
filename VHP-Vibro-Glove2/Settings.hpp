// SPDX-License-Identifier: AGPL-3.0-or-later


#ifndef SETTINGS_HPP_
#define SETTINGS_HPP_

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
  uint32_t samplerate = 46875;
  uint32_t stimfreq = 250;
  uint32_t stimduration = 1000;
  uint32_t cycleperiod = 8000;
  uint32_t pauzecycleperiod = 3;
  uint32_t pauzedcycles = 2;
  uint16_t jitter = 0;
  uint32_t vol_amplitude = 278;
  bool test_mode = true;
} g_settings;

#endif

