// SPDX-License-Identifier: AGPL-3.0-or-later

#include "pwm_sleeve.h"
#include "board_defs.h"
#include "battery_monitor.h"

#include "cpp/std_shim.h"

#include "BleComm.hpp"
#include "SStream.hpp"

using namespace audio_tactile;
// Output sequence for board Apollo84 hardware
//int order_pairs[8] = {0, 3, 4, 5, 11, 9, 8, 6};

// Output sequence for Godef Hardware
const uint16_t order_pairs[8] = {4, 5, 6, 7, 8, 9, 10, 11}; 


SStream *p = 0;


bool g_ble_connected = false;
uint8_t g_volume = 75;

void setup() {
  nrf_gpio_cfg_output(kLedPinBlue);
    
    //8 channel mode
    p = new     SStream(
	    true,     //chan8
    	    46875,    //samplerate 
	    250,      //stimfreq
	    100,      //stimduration
	    1332,     //cycleperiod
	    5,        //pauzecycleperiod,
	    2,        //pauzedcycles
	    235,      //jitter
	    278,      //volume
	    false);   //test_mode

    //4 channel mode.
    /*
    p = new     SStream(
	    false,    //chan8
	    46875,    //samplerate 
	    250,      //stimfreq
	    100,      //stimduration
	    666,      //cycleperiod
	    5,        //pauzecycleperiod,
	    2,        //pauzedcycles
	    235,      //jitter
	    64,       //volume
	    false);   //test_mode
    */

    SleeveTactors.OnSequenceEnd(OnPwmSequenceEnd);
    SleeveTactors.Initialize();
    nrf_gpio_pin_set(kLedPinBlue);
    
    SleeveTactors.SetUpsamplingFactor(1);

    // Warning: issue only in Arduino. When using StartPlayback() it crashes.
    // Looks like NRF_PWM0 module is automatically triggered, and triggering it
    // again here crashes ISR. Temporary fix is to only use nrf_pwm_task_trigger
    // for NRF_PWM1 and NRF_PWM2. To fix might need a nRF52 driver update.
    nrf_pwm_task_trigger(NRF_PWM1, NRF_PWM_TASK_SEQSTART0);
    nrf_pwm_task_trigger(NRF_PWM2, NRF_PWM_TASK_SEQSTART0);


    PuckBatteryMonitor.InitializeLowVoltageInterrupt();
    PuckBatteryMonitor.OnLowBatteryEventListener(low_battery_warning);

    BleCom.Init("F2Heal VHP", OnBleEvent);
    
    nrf_gpio_pin_clear(kLedPinBlue);
    
}


void OnPwmSequenceEnd() {
    p->next_sample_frame();

    for(uint32_t i = 0; i < p->channels(); i++) 
	SleeveTactors.UpdateChannel(order_pairs[i], p->chan_samples(i));
    
}

void loop() {
  uint16_t battery = PuckBatteryMonitor.MeasureBatteryVoltage();
  float converted = PuckBatteryMonitor.ConvertBatteryVoltageToFloat(battery);
  Serial.print("Battery voltage: ");
  Serial.println(converted);
  delay(120000);
}

void low_battery_warning() {
  nrf_gpio_pin_set(kLedPinBlue);  
  Serial.print("Low voltage trigger: ");
  Serial.println(PuckBatteryMonitor.GetEvent());
  // "0" event means that battery voltage is below reference voltage (3.5V)
  // "1" event means above.
}


void OnBleEvent() {
  switch (BleCom.event()) {
    case BleEvent::kConnect:
      Serial.println("BLE: Connected.");
      g_ble_connected = true;
      break;
    case BleEvent::kDisconnect:
      Serial.println("BLE: Disconnected.");
      g_ble_connected = false;
      break;
    case BleEvent::kInvalidMessage:
      Serial.println("BLE: Invalid message.");
      break;
    case BleEvent::kMessageReceived:
      HandleMessage(BleCom.rx_message());
      break;
  }
}

void HandleMessage(const Message& message) {
  switch (message.type()) {
  case MessageType::kVolume:
    Serial.println("Message: Volume.");
    message.ReadVolume(&g_volume);
    Serial.print("Volume: ");
    Serial.println(g_volume);
    break;
  case MessageType::kGetVolume:
    Serial.println("Message: GetVolume.");
    BleCom.tx_message().WriteVolume(g_volume);
    BleCom.SendTxMessage();
    break;

  default:
    Serial.println("Unhandled message.");
    break;
  }
}
