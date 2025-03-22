# Change log

## 1.3.0 - 2025-03-22

  * Add new webui: f2heal_webui_v2.html. See [Usage.md](doc/Usage.md).

## 1.2.0 - 2024-06-02

  * Add TTL control. See [Usage.md](doc/Usage.md) for details.

## 1.1.1 - 2024-05-21

 * Fix regression in mirrored hand stimulation for `chan8 = false`

## 1.1.0 - 2024-02-16

* Change to channel shuffle so that consecutive stimulation of the
  same channel becomes possible (F2Heal meeting 2024-02-11)

## 1.0.1 - 2024-01-14

Bugfix release, upgrade advised.

* bugfix next_sample_frame()

## 1.0.0 - 2024-01-06

* Rewrite of synthesis core, avoiding glithes in generated waves
* Removed dependency on audio-to-tactile library
* Ensure compatibility with adafruit:nrf52 1.6.0
* Removed UI.hpp


