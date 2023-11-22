// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef SETTINGS_HPP_
#define SETTINGS_HPP_

/**
 * Default stream settings.
 *
 * These settins will be used when the device is powered up.
 */
struct Settings {
  bool chan8 = true;
  uint32_t samplerate = 46875;
  uint32_t stimfreq = 250;
  uint32_t stimduration = 100;
  uint32_t cycleperiod = 1332;
  uint32_t pauzecycleperiod = 5;
  uint32_t pauzedcycles = 2;;
  uint16_t jitter = 235;
  uint32_t vol_amplitude = 278;
  bool test_mode = false;

  const uint32_t default_channels = 8;
  const bool     start_stream_on_power_on = false;
  
} g_settings;

#endif

