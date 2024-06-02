// SPDX-License-Identifier: AGPL-3.0-or-later
#include "src/PwmTactor.hpp"
#include "src/BatteryMonitor.hpp"


#include "src/BleComm.hpp"
#include "src/SStream.hpp"
#include "src/Settings.hpp"

using namespace audio_tactile;


SStream *g_stream = 0;

bool g_ble_connected = false;
bool g_running = false;
uint8_t g_volume = 75;
uint16_t g_volume_lvl = g_volume * g_settings.vol_amplitude / 100;
uint64_t g_running_since = 0;

void setup() {
    
    
    nrf_gpio_cfg_output(kLedPinBlue);
    nrf_gpio_cfg_output(kLedPinGreen);  
    nrf_gpio_pin_set(kLedPinBlue);
    
    PwmTactor.OnSequenceEnd(OnPwmSequenceEnd);
    PwmTactor.Initialize();
    
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

    // Configure button to toggle stream
    // Set pin as inputs with an internal pullup.
    nrf_gpio_cfg_input(kTactileSwitchPin, NRF_GPIO_PIN_PULLUP);
    attachInterrupt(kTactileSwitchPin_nrf, ToggleStream, RISING);

    //Configure TTL1 input , options:
    //NRF_GPIO_PIN_NOPULL     Pin pull-up resistor disabled.  
    //NRF_GPIO_PIN_PULLDOWN   Pin pull-down resistor enabled. 
    //NRF_GPIO_PIN_PULLUP     Pin pull-up resistor enabled.   
    nrf_gpio_cfg_input(kTTL1Pin, NRF_GPIO_PIN_NOPULL);

    //Attach interrupt:
    //LOW to trigger the interrupt whenever the pin is low,
    //CHANGE to trigger the interrupt whenever the pin changes value
    //RISING to trigger when the pin goes from low to high,
    //FALLING for when the pin goes from high to low.
    //HIGH to trigger the interrupt whenever the pin is high. (Supported?)
    attachInterrupt(digitalPinToInterrupt(kTTL1Pin), StartStream, LOW);

    //TTl2 input
    nrf_gpio_cfg_input(kTTL2Pin, NRF_GPIO_PIN_NOPULL);
    attachInterrupt(digitalPinToInterrupt(kTTL2Pin), StopStream, LOW);
    
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

volatile int g_starts = 0;
volatile int g_stops = 0;


void loop() {
    Serial.print("Starts: ");
    Serial.print(g_starts);
    Serial.print(", stops: ");
    Serial.println(g_stops);

    delay(5000);
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



void StartStream()
{
    g_starts++;
    
    if(!g_running)
	ToggleStream();
}

void StopStream()
{
    g_stops++;
    
    if(g_running)
	ToggleStream();
}



void ToggleStream() {
    
    if(g_running) {
	g_running = false;    
	nrf_gpio_pin_clear(kLedPinGreen);    
	Serial.println("Stream is running. Stopping.");
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
    
    if(g_ble_connected) {
	SendStatus();    
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
	break;
    case MessageType::k8Channel:
	message.Read(&g_settings.chan8);
	Serial.print("Message 8 Channel: ");
	Serial.println(g_settings.chan8);
	break;
    case MessageType::kStimFreq:
	message.Read(&g_settings.stimfreq);
	Serial.print("Message StimFreq:");
	Serial.println(g_settings.stimfreq);
	break;
    case MessageType::kStimDur:
	message.Read(&g_settings.stimduration);
	Serial.print("Message StimDur:");
	Serial.println(g_settings.stimduration);
	break;
    case MessageType::kCyclePeriod:
	message.Read(&g_settings.cycleperiod);
	Serial.print("Message CyclePeriod:");
	Serial.println(g_settings.cycleperiod);
	break;
    case MessageType::kPauzeCyclePeriod:
	message.Read(&g_settings.pauzecycleperiod);
	Serial.print("Message PauzeCyclePeriod:");
	Serial.println(g_settings.pauzecycleperiod);
	break;
    case MessageType::kPauzedCycles:
	message.Read(&g_settings.pauzedcycles);
	Serial.print("Message PauzeCycles:");
	Serial.println(g_settings.pauzedcycles);
	break;
    case MessageType::kJitter:
	message.Read(&g_settings.jitter);
	Serial.print("Message Jitter:");
	Serial.println(g_settings.jitter);
	break;
    case MessageType::kTestMode:
	message.Read(&g_settings.test_mode);
	Serial.print("Message TestMode:");
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

// avoid linker error:
// stl_vector.h undefined reference to std::__throw_length_error(char const*)
namespace std {
    void __throw_length_error( char const*e ) {
	Serial.print("Length Error :"); Serial.println(e);
	while (true) {}	
    }
}

