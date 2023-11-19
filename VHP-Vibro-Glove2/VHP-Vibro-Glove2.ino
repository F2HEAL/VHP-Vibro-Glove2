// SPDX-License-Identifier: AGPL-3.0-or-later

#include "pwm_sleeve.h"
#include "board_defs.h"
#include "battery_monitor.h"
#include "ui.h"

#include "cpp/std_shim.h"

#include "BleComm.hpp"
#include "SStream.hpp"
#include "Settings.hpp"

using namespace audio_tactile;
// Output sequence for board Apollo84 hardware
//int order_pairs[8] = {0, 3, 4, 5, 11, 9, 8, 6};

// Output sequence for Godef Hardware
const uint16_t order_pairs[8] = {4, 5, 6, 7, 8, 9, 10, 11}; 


SStream *g_stream = 0;
uint16_t g_silence[8];


bool g_ble_connected = false;
bool g_running = false;
uint8_t g_volume = 75;
uint64_t g_running_since = 0;

void setup() {
  nrf_gpio_cfg_output(kLedPinBlue);
  nrf_gpio_cfg_output(kLedPinGreen);  
  nrf_gpio_pin_set(kLedPinBlue);

  SleeveTactors.OnSequenceEnd(OnPwmSequenceEnd);
  SleeveTactors.Initialize();
    
  SleeveTactors.SetUpsamplingFactor(1);

  // Warning: issue only in Arduino. When using StartPlayback() it crashes.
  // Looks like NRF_PWM0 module is automatically triggered, and triggering it
  // again here crashes ISR. Temporary fix is to only use nrf_pwm_task_trigger
  // for NRF_PWM1 and NRF_PWM2. To fix might need a nRF52 driver update.
  nrf_pwm_task_trigger(NRF_PWM1, NRF_PWM_TASK_SEQSTART0);
  nrf_pwm_task_trigger(NRF_PWM2, NRF_PWM_TASK_SEQSTART0);

  PuckBatteryMonitor.InitializeLowVoltageInterrupt();
  PuckBatteryMonitor.OnLowBatteryEventListener(LowBatteryWarning);

  BleCom.Init("F2Heal VHP", OnBleEvent);

  SetSilence();

  DeviceUi.Initialize(kTactileSwitchPin);
  DeviceUi.OnUiEventListener(ToggleStream);
  
  nrf_gpio_pin_clear(kLedPinBlue);
  nrf_gpio_pin_clear(kLedPinGreen);  
}


void SetSilence() {
  for(uint32_t i=0; i < 8; i++) {
    g_silence[i] = g_volume * g_settings.vol_amplitude / 100;
  }
}

void OnPwmSequenceEnd() {
  if(g_running) {
    g_stream->next_sample_frame();
    
    for(uint32_t i = 0; i < g_stream->channels(); i++) 
      SleeveTactors.UpdateChannel(order_pairs[i], g_stream->chan_samples(i));
  } else {
    for(uint32_t i = 0; i < g_settings.default_channels; i++)
      SleeveTactors.UpdateChannel(order_pairs[i], g_silence);
  }    
}

void loop() {
  // Output battery voltage via serial (debugging)
  uint16_t battery = PuckBatteryMonitor.MeasureBatteryVoltage();
  float converted = PuckBatteryMonitor.ConvertBatteryVoltageToFloat(battery);
  Serial.print("Battery voltage: ");
  Serial.println(converted);
  delay(120000);
}

void LowBatteryWarning() {
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

void ToggleStream() {
  // if(!g_ble_connected) {
  //   Serial.println("BLE: Not connected. Cannot toggle Stream.");
  //   return;
  // }

  if(g_running) {
    nrf_gpio_pin_clear(kLedPinGreen);    
    Serial.println("Stream already running. Stopping.");
    g_running = false;
    delete g_stream;
  } else {
    nrf_gpio_pin_set(kLedPinGreen);
    Serial.println("Starting Stream.");
    g_stream = new SStream(g_settings.chan8,
			   g_settings.samplerate,
			   g_settings.stimfreq,
			   g_settings.stimduration,
			   g_settings.cycleperiod,
			   g_settings.pauzecycleperiod,
			   g_settings.pauzedcycles,
			   g_settings.jitter,
			   g_volume * g_settings.vol_amplitude / 100,
			   g_settings.test_mode);
    g_running = true;
    g_running_since = millis(); 
  }

}

void SendStatus() {
  Serial.println("Message: GetStatus.");
  uint16_t battery_voltage_uint16 = PuckBatteryMonitor.MeasureBatteryVoltage();
  float battery_voltage_float = PuckBatteryMonitor.ConvertBatteryVoltageToFloat(battery_voltage_uint16);

  uint64_t running_period = 0;
  if(g_running) {
    running_period = millis() - g_running_since;
  }
  
  BleCom.tx_message().WriteStatus(g_running, running_period, battery_voltage_float);
  BleCom.SendTxMessage();
}

void HandleMessage(const Message& message) {
  
  
  switch (message.type()) {
  case MessageType::kVolume:
    message.Read(&g_volume);
    SetSilence();    
    Serial.print("Message Volume: ");
    Serial.println(g_volume);
    break;
  case MessageType::kGetVolume:
    Serial.println("Message: GetVolume.");
    BleCom.tx_message().WriteVolume(g_volume);
    BleCom.SendTxMessage();
    break;
  case MessageType::kToggle:
    Serial.println("Message: Toggle.");
    ToggleStream();
    SendStatus();
    break;
  case MessageType::k8Channel:
    message.Read(&g_settings.chan8);
    Serial.print("Message 8 Channel: ");
    Serial.println(g_settings.chan8);
    break;
  case MessageType::kStimFreq:
    message.Read(&g_settings.stimfreq);
    Serial.print("Message  StimFreq:");
    Serial.println(g_settings.stimfreq);
    break;
  case MessageType::kStimDur:
    message.Read(&g_settings.stimduration);
    Serial.print("Message  StimDur:");
    Serial.println(g_settings.stimduration);
    break;
  case MessageType::kCyclePeriod:
    message.Read(&g_settings.cycleperiod);
    Serial.print("Message  CyclePeriod:");
    Serial.println(g_settings.cycleperiod);
    break;
  case MessageType::kPauzeCyclePeriod:
    message.Read(&g_settings.pauzecycleperiod);
    Serial.print("Message  PauzeCyclePeriod:");
    Serial.println(g_settings.pauzecycleperiod);
    break;
  case MessageType::kPauzedCycles:
    message.Read(&g_settings.pauzedcycles);
    Serial.print("Message  PauzeCycles:");
    Serial.println(g_settings.pauzedcycles);
    break;
  case MessageType::kJitter:
    message.Read(&g_settings.jitter);
    Serial.print("Message  Jitter:");
    Serial.println(g_settings.jitter);
    break;
  case MessageType::kTestMode:
    message.Read(&g_settings.test_mode);
    Serial.print("Message  TestMode:");
    Serial.println(g_settings.test_mode);
    break;
  case MessageType::kGetSettingsBatch:
    Serial.println("Message: GetSettings.");
    BleCom.tx_message().WriteSettings(g_settings);
    BleCom.SendTxMessage();
    break;
  case MessageType::kGetStatusBatch:
    SendStatus();
    break;    
  default:
    Serial.println("Unhandled message.");
    break;
  }
}
