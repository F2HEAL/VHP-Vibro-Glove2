// SPDX-License-Identifier: AGPL-3.0-or-later
// 20250507 bepg

const char* FIRMWARE_VERSION = "SC_1_0_3";  // Using const char* instead of String


#include "src/PwmTactor.hpp"
#include "src/BatteryMonitor.hpp"
#include "src/BleComm.hpp"
#include "src/SStream.hpp"
#include "src/Settings.hpp"

using namespace audio_tactile;

SStream *g_stream = 0;
bool g_ble_connected = false;
bool g_running = false;
uint8_t g_volume = 25;
uint16_t g_volume_lvl = g_volume * g_settings.vol_amplitude / 100;
uint64_t g_running_since = 0;

String serialBuffer = "";


void setup() {
    nrf_gpio_cfg_output(kLedPinBlue);
    nrf_gpio_cfg_output(kLedPinGreen);  
    nrf_gpio_pin_set(kLedPinBlue);
    
    PwmTactor.OnSequenceEnd(OnPwmSequenceEnd);
    PwmTactor.Initialize();
    
    nrf_pwm_task_trigger(NRF_PWM1, NRF_PWM_TASK_SEQSTART0);
    nrf_pwm_task_trigger(NRF_PWM2, NRF_PWM_TASK_SEQSTART0);
    
    PuckBatteryMonitor.InitializeLowVoltageInterrupt();
    PuckBatteryMonitor.OnLowBatteryEventListener(LowBatteryWarning);

    // Create BLE name with version (using char array instead of String)
    char bleName[32];  // BLE names have max 31 chars + null terminator
    snprintf(bleName, sizeof(bleName), "F2Heal VHP v%s", FIRMWARE_VERSION);
    
    BleCom.Init(bleName, OnBleEvent);
    
    SetSilence();

    nrf_gpio_cfg_input(kTactileSwitchPin, NRF_GPIO_PIN_PULLUP);
    attachInterrupt(kTactileSwitchPin_nrf, ToggleStream, RISING);

    nrf_gpio_cfg_input(kTTL1Pin, NRF_GPIO_PIN_NOPULL);
    attachInterrupt(kTTL1Pin_nrf, StartStream, RISING);
    attachInterrupt(kTTL1Pin_nrf, StopStream, FALLING);
    
    nrf_gpio_pin_clear(kLedPinBlue);
    nrf_gpio_pin_clear(kLedPinGreen);  
    
    if(g_settings.start_stream_on_power_on) {
        ToggleStream();
    }
}

void SetSilence() {
    g_volume_lvl = g_volume * g_settings.vol_amplitude / 100;
}

void OnPwmSequenceEnd() {
    if(g_running) {
        g_stream->next_sample_frame();
        
        const auto active_channel = g_stream->current_active_channel();
        
        for(uint32_t channel = 0; channel < g_stream->channels(); channel++)
            if(channel==active_channel) {
                uint16_t* cp = PwmTactor.GetChannelPointer(channel);
                g_stream->set_chan_samples(cp, channel);

                if(!g_settings.chan8) {
                    uint16_t* cp = PwmTactor.GetChannelPointer(7-channel);
                    g_stream->set_chan_samples(cp, channel);
                }
            } else {
                PwmTactor.SilenceChannel(channel, g_volume_lvl);
                if(!g_settings.chan8)
                    PwmTactor.SilenceChannel(7-channel, g_volume_lvl);
            }
    } else {
        for(uint32_t i = 0; i < g_settings.default_channels; i++)
            PwmTactor.SilenceChannel(i, g_volume_lvl);
    }    
}

void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();

    if (cmd == '\n' || cmd == '\r') {
      // End of a command
      if (serialBuffer.length() > 0) {
        processSerialCommand(serialBuffer);
        serialBuffer = "";
      }
    }
    else {
      serialBuffer += cmd;
    }
  }
}

void processSerialCommand(String cmd) {
  if (cmd.length() == 0) return;

  if (cmd.length() == 1) {
    char c = cmd.charAt(0);
    
    if (c == '1') {
      StartStream();
      Serial.write('1');
    }
    else if (c == '0') {
      StopStream();
      Serial.write('0');
    }
    else if (c == 'T') {
      ToggleStream();
      Serial.write('T');
    }
    else if (c == 'L') {
      Serial.write('L');
    }
    else if (c == 'S') {  // Added 'S' command to get version
      Serial.print("FW Version: ");
      Serial.println(FIRMWARE_VERSION);
    }
    else if (c == 'X') {  // Added 'V' command to get version
      Serial.print("V");
      Serial.print(g_volume);
      Serial.print(" F");
      Serial.print(g_settings.stimfreq);
      Serial.print(" D");
      Serial.print(g_settings.stimduration);
      Serial.print(" Y");
      Serial.print(g_settings.cycleperiod);
      Serial.print(" P");
      Serial.print(g_settings.pauzecycleperiod);
      Serial.print(" Q");
      Serial.print(g_settings.pauzedcycles);
      Serial.print(" J");
      Serial.print(g_settings.jitter);
      Serial.print(" M");
      Serial.print(g_settings.test_mode);
    }
    else {
      Serial.print("Unknown: ");
      Serial.println(c);
    }
  }
  else {
    // Multi-character command, e.g. V128 or F250
    char commandType = cmd.charAt(0);
    int value = cmd.substring(1).toInt();

    switch (commandType) {
      case 'V':
        g_volume = constrain(value, 0, 100);
        Serial.print("Volume set to ");
        Serial.println(g_volume);
        break;

      case 'F':
        g_settings.stimfreq = constrain(value, 1, 400);
        Serial.print("'stimfreq' set to ");
        Serial.println(g_settings.stimfreq);
        break;

      case 'D':
        g_settings.stimduration = constrain(value, 1, 65535);
        Serial.print("'stimduration' set to ");
        Serial.println(g_settings.stimduration);
        break;

      case 'Y':
        g_settings.cycleperiod = constrain(value, 1, 65535);
        Serial.print("'cycleperiod' set to ");
        Serial.println(g_settings.cycleperiod);
        break;

      case 'P':
        g_settings.pauzecycleperiod = constrain(value, 0, 100);
        Serial.print("'pauzecycleperiod' set to ");
        Serial.println(g_settings.pauzecycleperiod);
        break;

      case 'Q':
        g_settings.pauzedcycles = constrain(value, 0, 100);
        Serial.print("'pauzedcycles' set to ");
        Serial.println(g_settings.pauzedcycles);
        break;

      case 'J':
        g_settings.jitter = constrain(value, 0, 1000);
        Serial.print("'jitter' set to ");
        Serial.println(g_settings.jitter);
        break;

      case 'M':
        g_settings.test_mode = (value != 0);
        Serial.print("'test_mode' set to ");
        Serial.println(g_settings.test_mode ? "true" : "false");
        break;

      case 'C':
        g_settings.single_channel = constrain(value, 0, 8);
        Serial.print("'single_channel' set to ");
        Serial.println(g_settings.single_channel);
        break;

      default:
        Serial.print("Unknown command: ");
        Serial.println(cmd);
        break;
    }
  }
}


void LowBatteryWarning() {
    nrf_gpio_pin_set(kLedPinBlue);  
}

void OnBleEvent() {
    switch (BleCom.event()) {
    case BleEvent::kConnect:
        g_ble_connected = true;
        break;
    case BleEvent::kDisconnect:
        g_ble_connected = false;
        break;
    case BleEvent::kInvalidMessage:
        break;
    case BleEvent::kMessageReceived:
        HandleMessage(BleCom.rx_message());
        break;
    }
}

void StartStream() {
    if(!g_running) ToggleStream();
}

void StopStream() {
    if(g_running) ToggleStream();
}

volatile unsigned long g_last_toggle = 0;

void ToggleStream() {
    auto now = millis();
    if(now - g_last_toggle < 250) return;
    g_last_toggle = now;
    
    if(g_running) {
        g_running = false;    
        nrf_gpio_pin_clear(kLedPinGreen);    
        delete g_stream;
    } else {
        nrf_gpio_pin_set(kLedPinGreen);
        g_stream = new SStream(g_settings.chan8,
                     g_settings.samplerate,
                     g_settings.stimfreq,
                     g_settings.stimduration,
                     g_settings.cycleperiod,
                     g_settings.pauzecycleperiod,
                     g_settings.pauzedcycles,
                     g_settings.jitter,
                     g_volume * g_settings.vol_amplitude / 100,
                     g_settings.test_mode,
                     g_settings.single_channel);
        g_running = true;
        g_running_since = millis(); 
    }
    
    if(g_ble_connected) {
        SendStatus();    
    }
}

void SendStatus() {
    uint16_t battery_voltage_uint16 = PuckBatteryMonitor.MeasureBatteryVoltage();
    float battery_voltage_float = PuckBatteryMonitor.ConvertBatteryVoltageToFloat(battery_voltage_uint16);
    
    uint64_t running_period = 0;
    if(g_running) {
        running_period = millis() - g_running_since;
    }
    
// Debug log to check values before sending
    Serial.print("FIRMWARE_VERSION: ");
    Serial.println(FIRMWARE_VERSION);
    Serial.print("default_parameter_settings: ");
    Serial.println(default_parameter_settings);

    BleCom.tx_message().WriteStatus(g_running, running_period, battery_voltage_float, FIRMWARE_VERSION, default_parameter_settings);
    BleCom.SendTxMessage();
}

void HandleMessage(const Message& message) {
    switch (message.type()) {
    case MessageType::kVolume:
        message.Read(&g_volume);
        SetSilence();    
        break;
    case MessageType::kGetVolume:
        BleCom.tx_message().WriteVolume(g_volume);
        BleCom.SendTxMessage();
        break;
    case MessageType::kToggle:
        ToggleStream();
        break;
    case MessageType::k8Channel:
        message.Read(&g_settings.chan8);
        break;
    case MessageType::kStimFreq:
        message.Read(&g_settings.stimfreq);
        break;
    case MessageType::kStimDur:
        message.Read(&g_settings.stimduration);
        break;
    case MessageType::kCyclePeriod:
        message.Read(&g_settings.cycleperiod);
        break;
    case MessageType::kPauzeCyclePeriod:
        message.Read(&g_settings.pauzecycleperiod);
        break;
    case MessageType::kPauzedCycles:
        message.Read(&g_settings.pauzedcycles);
        break;
    case MessageType::kJitter:
        message.Read(&g_settings.jitter);
        break;
    case MessageType::kSingleChannel:
        message.Read(&g_settings.single_channel);
        break;    
    case MessageType::kTestMode:
        message.Read(&g_settings.test_mode);
        break;
    case MessageType::kGetSettingsBatch:
        BleCom.tx_message().WriteSettings(g_settings);
        BleCom.SendTxMessage();
        break;
    case MessageType::kGetStatusBatch:
        SendStatus();
        break;
    case MessageType::kGetVersion:  // Added case for version request
        BleCom.tx_message().WriteVersion(FIRMWARE_VERSION);
        BleCom.SendTxMessage();
        break;   
    default:
        break;
    }
}

namespace std {
    void __throw_length_error(char const* e) {
        while (true) {}    
    }
}	
