// Copyright 2021-2022 Google LLC
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
// Message class for representing serializable commands.
//
// `Message` is a unified representation for sending commands and information
// between devices (puck, sleeve, web app, ...) and also useful within a device
// to communicate between event handlers and the main loop.
//
// Although Messages may interact with hardware, this library should itself be
// hardware agnostic. This improves compatibility across devices and enables
// this code to be reused in other contexts, like emscripten and Android NDK.

#ifndef MESSAGE_HPP_
#define MESSAGE_HPP_

#include <stdint.h>

#include "att/Slice.hpp"
#include "att/Serialize.hpp"

#include "Settings.hpp"

namespace audio_tactile {

// Types of messages. Append new op codes without changing old ones, as they
// ensure compatibility when communicating with external devices. Type values
// must be between 0 and 255.
    enum class MessageType {
	kNone = 0,
	kVolume = 1,
	kToggle = 2,
	kChannelTest = 3,
	k8Channel = 4,
	kStimFreq = 5,
	kStimDur = 6,
	kCyclePeriod = 7,
	kPauzeCyclePeriod = 8,
	kPauzedCycles = 9,
	kJitter = 10,
	kTestMode = 11,
	kStatusBatch = 12,
	kGetStatusBatch = 13,
	kGetVolume = 14,
	kSettingsBatch = 15, 
	kGetSettingsBatch = 16,
	kSingleChannel = 17
    };

// Recipients of messages -- Not used, can be removed
    enum class MessageRecipient {
	kNone,
	kAll,
	kPuck,
	kSleeve,
	kSlimBoard,
	kConnectedBleDevice,
    };

    class Message {
    public:
	enum {
	    // Number of header bytes.
	    kHeaderSize = 4,
	    // Max number of payload bytes.
	    kMaxPayloadSize = 128,
	    // Max message size = header size + max payload size.
	    kMaxMessageSize = kHeaderSize + kMaxPayloadSize,
	    // Start-of-frame code for first byte of serial header.
	    kPacketStart = 200,
	};

	// Gets raw data pointer to the full message, including header.
	const uint8_t* data() const { return bytes_; }
	uint8_t* data() { return bytes_; }
	// Gets number of bytes in the message, considering the payload size.
	int size() const { return kHeaderSize + payload_size(); }

	// Set a serial header with start byte and recipient.
	void SetHeader(MessageRecipient recipient) {
	    bytes_[0] = kPacketStart;
	    bytes_[1] = static_cast<uint8_t>(recipient);
	}

	// Computes and sets checksum header for BLE. This method should be called
	// after all other data is set.
	void SetBleHeader(){
	    ::LittleEndianWriteU16(ComputeChecksum(), bytes_);
	}

	// Verifies the checksum for a message with BLE header. Returns true if valid.
	bool VerifyChecksum() {
	    return ::LittleEndianReadU16(bytes_) == ComputeChecksum(); 
	}


	// The message recipient. Note, this field is only valid if it has been set
	// with SetHeader().
	MessageRecipient recipient() const {
	    return static_cast<MessageRecipient>(bytes_[1]);
	}

	// The message type, e.g. kTurnOnAmplifiers.
	MessageType type() const { return static_cast<MessageType>(bytes_[2]); }
	void set_type(MessageType type) { bytes_[2] = static_cast<uint8_t>(type); }

	// The message payload.
	Slice<const uint8_t> payload() const {
	    return {bytes_ + kHeaderSize, payload_size()};
	}
    
	Slice<uint8_t> payload() { return {bytes_ + kHeaderSize, payload_size()}; }
    
	template <int kSize>
	void set_payload(Slice<const uint8_t, kSize> new_payload) {
	    static_assert(kSize == kDynamic || kSize <= kMaxPayloadSize,
			  "Payload size must be less than kMaxPayloadSize");
	    bytes_[3] = static_cast<uint8_t>(new_payload.size());
	    payload().CopyFrom(new_payload);
	}

	// Methods for writing and reading messages of predefined types.
	// The Write* methods set the type and payload, but not the first two header
	// bytes. SetHeader() or SetBleHeader() should be called after one of these to
	// form a complete message.
	//
	// The Read* methods assume the type has already been checked, and only look
	// at the payload. In all cases they return true on success, false on failure.

	// Writes a kVolume message to send volume.
	void WriteVolume(uint8_t volume) {
	    uint8_t volume_bytes[1] = {volume};
	    SetTypeAndPayload(MessageType::kVolume, Slice<uint8_t,1>(volume_bytes));
	}

	// Writes a kSettings message
	void WriteSettings(const Settings& settings) {
	    uint8_t* dest = bytes_ + kHeaderSize;

	    *dest = settings.chan8 ? 1 : 0; dest++;
	    ::LittleEndianWriteU32(settings.stimfreq, dest); dest += 4;
	    ::LittleEndianWriteU32(settings.stimduration, dest); dest += 4;
	    ::LittleEndianWriteU32(settings.cycleperiod, dest); dest += 4;
	    ::LittleEndianWriteU32(settings.pauzecycleperiod, dest); dest += 4;
	    ::LittleEndianWriteU32(settings.pauzedcycles, dest); dest += 4;
	    ::LittleEndianWriteU32(settings.jitter, dest); dest += 4;
	    *dest = settings.test_mode ? 1 : 0; dest++;	    
	    ::LittleEndianWriteU32(settings.single_channel, dest); dest += 4;

	    
	    bytes_[3] = dest - (bytes_ + kHeaderSize);
	    set_type(MessageType::kSettingsBatch);
	}

	// Writes a kStatus message
	void WriteStatus(const bool running,
			 const uint64_t& running_since,
			 const float battery_voltage) {

    
	    uint8_t* dest = bytes_ + kHeaderSize;
    
	    *dest = running ? 1 : 0; dest++;
	    ::LittleEndianWriteU64(running_since, dest); dest += 8;
	    ::LittleEndianWriteF32(battery_voltage, dest); dest += 4;

	    bytes_[3] = dest - (bytes_ + kHeaderSize);
	    set_type(MessageType::kStatusBatch);
	}
  
  
	// Reads uint8 from a BLE message
	bool Read(uint8_t* v) const {
	    *v = *payload().data();
	    return true;
	}

	// Reads bool value from a BLE messag
	// Payload must equal to 1 for true
	bool Read(bool* b) const {
	    *b = *payload().data() == 1;
	    return true;
	}

	// Reads uint16 from a BLE message
	bool Read(uint16_t* v) const {
	    *v = ::LittleEndianReadU16(payload().data());
	    return true;
	}

  
	// Reads uint32 from a BLE message
	bool Read(uint32_t* v) const {
	    *v = ::LittleEndianReadU32(payload().data());
	    return true;
	}

    private:
	int payload_size() const { return bytes_[3]; }

	void SetTypeAndPayload(MessageType type, Slice<const uint8_t> payload) {
	    set_type(type);
	    set_payload(payload);
	}

	uint16_t ComputeChecksum() const {
	    return ::Fletcher16(bytes_ + 2, (kHeaderSize - 2) + payload_size(),
				/*init=*/1);
	}

	uint8_t bytes_[kHeaderSize + kMaxPayloadSize] =  {0};
    };

}  // namespace audio_tactile

#endif  // AUDIO_TO_TACTILE_SRC_CPP_MESSAGE_H_
