// Copyright 2021 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//
// BLE communication interface.
//
// Example use:
//   BleCom.Init("Tactile device", OnBleEvent);
//
//   void OnBleEvent() {
//     switch (BleCom.event()) {
//       case BleEvent::kMessageReceived:
//         HandleMessage(BleCom.rx_message());
//         break;
//       case BleEvent::kConnect:
//         Serial.println("BLE connected.");
//         break;
//     }
//   }

#ifndef BLE_COM_H_
#define BLE_COM_H_

#include <bluefruit.h>


#include "Message.hpp"

namespace audio_tactile {

  enum class BleEvent {
    kNone,
    kMessageReceived,
    kInvalidMessage,
    kConnect,
    kDisconnect,
  };
  
  static void OnBleConnect(uint16_t connection_handle);
  static void OnBleDisconnect(uint16_t connection_handle, uint8_t reason);
  static void OnBleUartRx(uint16_t connection_handle);
  
  class AudioTactileBleCom {
  public:
    AudioTactileBleCom(): event_fun_(nullptr), event_(BleEvent::kNone) {}

    // Initializes and begins BLE advertising.
    void Init(const char* device_name, void (*event_fun)()) {
      event_fun_ = event_fun;

      Bluefruit.autoConnLed(false);
      Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);
      Bluefruit.begin();
      Bluefruit.setTxPower(-8);
      Bluefruit.setName("F2Heal-BLE");
      Bluefruit.Periph.setConnectCallback(OnBleConnect);
      Bluefruit.Periph.setDisconnectCallback(OnBleDisconnect);

      ble_uart_.begin();
      ble_uart_.setRxCallback(OnBleUartRx, true);

      // Start the OTA (Over-the-air) DFU (Device Firmware Update) functionality.
      bledfu_.begin();

      // Start advertising.
      Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
      Bluefruit.Advertising.addTxPower();
      Bluefruit.Advertising.addService(ble_uart_);
      Bluefruit.ScanResponse.addName();
      // Start advertising again if disconnected.
      Bluefruit.Advertising.restartOnDisconnect(true);
      // To balance between quick discovery vs. battery cost, we first do "fast"
      // advertising with an interval of 20 ms for 30 s. Then we switch to slower
      // interval of 318.75 ms for another 270 s, then stop advertising. After that
      // point, BLE will no longer connect until the device is reset.
      //
      // Reference, Apple's recommendations for BLE advertising:
      // https://developer.apple.com/library/archive/qa/qa1931/_index.html
      Bluefruit.Advertising.setInterval(32, 510);  // Units of 0.625 ms.
      Bluefruit.Advertising.setFastTimeout(30);  // Units of seconds.
      Bluefruit.Advertising.start(300);  // Units of seconds.

      Serial.print("BleCom: Advertising as \"");
      Serial.print("F2Heal-BLE");
      Serial.println("\"");
    }


    // Gets the most recent event.
    BleEvent event() const { return event_; }

    // Gets Message that will be transmitted.
    Message& tx_message() { return tx_message_; }
    // Sends tx_message over BLE UART.
    void SendTxMessage() {
      tx_message_.SetBleHeader();
      ble_uart_.write(tx_message_.data(), tx_message_.size());
    }
  
    // Gets Message that was most recently received.
    Message& rx_message() { return rx_message_; }

    friend void OnBleConnect(uint16_t connection_handle);
    friend void OnBleDisconnect(uint16_t connection_handle, uint8_t reason);
    friend void OnBleUartRx(uint16_t connection_handle);

  private:
    // Reads a message from ble_uart_ into rx_message_ and updates event_.
    void ReadFromBleUart() {
      constexpr int kHeaderSize = Message::kHeaderSize;
      constexpr int kMaxPayloadSize = Message::kMaxPayloadSize;
      const int num_received_bytes = ble_uart_.available();
      uint8_t* dest = rx_message_.data();
      int payload_size;

      // Read from ble_uart_, writing directly into rx_message_. We first read the
      // 4-byte message header to get the payload size, then read the payload.
      //
      // When BLE receives a packet, BLEUart appears to buffer the whole packet and
      // then call the `OnBleUartRx()` callback above. Then, this function can
      // instantly read the whole packet without waiting. This has been reliable in
      // all tests with it so far, in which BLE transferred at most 20 bytes at a
      // time. I'd expect that with a sufficiently large transmission, NUS will
      // break it into multiple packets sent over multiple BLE time intervals. Then
      // this receiving code would need some streaming or waiting logic to get the
      // whole transmission. But this is complicated and not needed so far.
      if (num_received_bytes >= kHeaderSize &&  // Read the message header.
	  ble_uart_.read(dest, kHeaderSize) == kHeaderSize &&
	  dest[2] >= 1 &&  // Check type field.
	  (payload_size = dest[3]) <= kMaxPayloadSize &&  // Check size field.
	  num_received_bytes >= kHeaderSize + payload_size &&  // Read the payload.
	  ble_uart_.read(dest + kHeaderSize, payload_size) == payload_size &&
	  // Verify the checksum.
	  rx_message_.VerifyChecksum()) {
	event_ = BleEvent::kMessageReceived;
      } else {
	event_ = BleEvent::kInvalidMessage;
      }

      ble_uart_.flush();  // Flush in case a partial or failed message remains.
      event_fun_();
    }

    BLEUart ble_uart_;
    Message rx_message_;
    Message tx_message_;
    void (*event_fun_)();
    BleEvent event_;
    BLEDfu bledfu_;
  };

  static AudioTactileBleCom BleCom;

  static void OnBleConnect(uint16_t connection_handle) {
    BleCom.event_ = BleEvent::kConnect;
    BleCom.event_fun_();
  }

  static void OnBleDisconnect(uint16_t connection_handle, uint8_t reason) {
    BleCom.event_ = BleEvent::kDisconnect;
    BleCom.event_fun_();
  }

  static void OnBleUartRx(uint16_t connection_handle) {
    BleCom.ReadFromBleUart();
  }
  
}  // namespace audio_tactile

#endif  // BLE_COM_H_
