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
// Library for the Max14661 16:2 multiplexer (mux) from Maxim Integrated. This
// multiplexer has 16 inputs and 2 outputs. The connections between inputs and
// outputs can be set programmatically in any configuration.
//
// This driver is written for use with 12 vibrotactile channels in Slim Board
// for current sensing. The board has two 16:2 muxes, so for ease of
// integration, the driver behaves as there is one 32:2 mux.
//
// The datasheet is provided here:
// https://datasheets.maximintegrated.com/en/ds/MAX14661.pdf

#ifndef MAX14661_HPP_
#define MAX14661_HPP_

#include <stdint.h>


namespace {
    extern "C" {
	static volatile uint32_t *pincfg_reg(uint32_t pin) {
	    NRF_GPIO_Type *port = nrf_gpio_pin_port_decode(&pin);
	    return &port->PIN_CNF[pin];
	}

	void i2c_write(uint8_t addr, uint8_t data) {
	    static uint8_t tx_buf[2];
	    NRF_TWIM0->SHORTS = TWIM_SHORTS_LASTTX_STOP_Msk;

	    tx_buf[0] = addr;
	    tx_buf[1] = data;
	    NRF_TWIM0->TXD.MAXCNT = sizeof(tx_buf);
	    NRF_TWIM0->TXD.PTR = (uint32_t)&tx_buf[0];

	    NRF_TWIM0->EVENTS_STOPPED = 0;
	    NRF_TWIM0->TASKS_STARTTX = 1;
	    while (NRF_TWIM0->EVENTS_STOPPED == 0) {
	    }
	}

	uint8_t i2c_read(uint8_t addr) {
	    /* Arrays are static since the hardware uses them after the function exits. */
	    static uint8_t tx_buf[1];
	    static uint8_t rx_buf[1];
	    /* Enable shortcuts that starts a read right after a write and sends a stop
	     * condition after last TWI read.
	     */
	    NRF_TWIM0->SHORTS =
		TWIM_SHORTS_LASTTX_STARTRX_Msk | TWIM_SHORTS_LASTRX_STOP_Msk;
	    /* Transmit the address. */
	    tx_buf[0] = addr;
	    NRF_TWIM0->TXD.MAXCNT = sizeof(tx_buf);
	    NRF_TWIM0->TXD.PTR = (uint32_t)&tx_buf[0];

	    NRF_TWIM0->RXD.MAXCNT = 1;  /* Max number of bytes per transfer. */
	    NRF_TWIM0->RXD.PTR = (uint32_t)&rx_buf[0];  /* Point to RXD buffer. */

	    NRF_TWIM0->EVENTS_STOPPED = 0;
	    NRF_TWIM0->TASKS_STARTTX = 1;
	    while (NRF_TWIM0->EVENTS_STOPPED == 0) {
	    }

	    return rx_buf[0];
	}

	uint8_t *i2c_read_array(uint8_t addr, uint8_t size) {
	    /* Arrays are static since they are used after the function exits. */
	    static uint8_t tx_buf[1];
	    static uint8_t buffer[8];
	    /* Enable shortcuts that starts a read right after a write and sends a stop
	     * condition after last TWI read.
	     */
	    NRF_TWIM0->SHORTS =
		TWIM_SHORTS_LASTTX_STARTRX_Msk | TWIM_SHORTS_LASTRX_STOP_Msk;

	    tx_buf[0] = addr;
	    NRF_TWIM0->TXD.MAXCNT = sizeof(tx_buf);
	    NRF_TWIM0->TXD.PTR = (uint32_t)&tx_buf[0];

	    /* Load the data pointer into the TWI registers. */
	    NRF_TWIM0->RXD.MAXCNT = size; /* Max number of bytes per transfer. */
	    NRF_TWIM0->RXD.PTR = (uint32_t)&buffer; /* Point to RXD buffer. */

	    /* Start read sequence. Note that it uses starttx, not start RX. */
	    NRF_TWIM0->EVENTS_STOPPED = 0;
	    NRF_TWIM0->TASKS_STARTTX = 1;

	    /* Wait for the device to finish up. Currently, there is no time out so this
	     * can go forever if somthing is wrong.
	     */
	    while (NRF_TWIM0->EVENTS_STOPPED == 0) {
	    }

	    return buffer;
	}

	void i2c_init(uint8_t scl, uint8_t sda, uint8_t device_addr) {
	    *pincfg_reg(scl) =
		((uint32_t)GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos) |
		((uint32_t)GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
		((uint32_t)GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
		((uint32_t)GPIO_PIN_CNF_DRIVE_S0D1 << GPIO_PIN_CNF_DRIVE_Pos) |
		((uint32_t)GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);

	    *pincfg_reg(sda) =
		((uint32_t)GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos) |
		((uint32_t)GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
		((uint32_t)GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
		((uint32_t)GPIO_PIN_CNF_DRIVE_S0D1 << GPIO_PIN_CNF_DRIVE_Pos) |
		((uint32_t)GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);

	    NRF_TWIM0->PSEL.SCL = scl;
	    NRF_TWIM0->PSEL.SDA = sda;

	    NRF_TWIM0->ADDRESS = device_addr;
	    NRF_TWIM0->FREQUENCY = TWIM_FREQUENCY_FREQUENCY_K400
		<< TWIM_FREQUENCY_FREQUENCY_Pos;
	    NRF_TWIM0->SHORTS = 0;

	    NRF_TWIM0->ENABLE = TWIM_ENABLE_ENABLE_Enabled << TWIM_ENABLE_ENABLE_Pos;
	}

	void i2c_write_array(const uint8_t *data, uint16_t size) {
	    NRF_TWIM0->SHORTS = TWIM_SHORTS_LASTTX_STOP_Msk;

	    NRF_TWIM0->TXD.MAXCNT = size;
	    NRF_TWIM0->TXD.PTR = (uint32_t)data;

	    NRF_TWIM0->EVENTS_STOPPED = 0;
	    NRF_TWIM0->TASKS_STARTTX = 1;
	    while (NRF_TWIM0->EVENTS_STOPPED == 0) {
	    }
	}
	
	
    }
}


namespace audio_tactile {

    class Max14661 {
    public:
	enum {
	    // The I2C address of the multiplexer is set by A0 and A1 pins, which are
	    // physical pins that can be connected to ground (low) or Vdd (high)
	    // The byte address is determined by binary: 10011[A1][A0]
	    kMax14661Address1 = 0x4C,  // A0: 0, A1: 0. b1001100
	    kMax14661Address2 = 0x4E,  // A0: 0, A1: 1. b1001110
	    // Pin definitions.
	    kMuxEnable = 2,
	    kSclPin = 25,
	    kSdaPin = 24
	};
	// Initializes the two muxes.
	void Initialize() {
	    // Initializes the I2C bus.
	    i2c_init(kSclPin, kSdaPin, kMax14661Address1);

	    // Sets mux enable pin as output (both muxes are connected to same pin).
	    nrf_gpio_cfg_output(kMuxEnable);

	    // Enables the muxes.
	    nrf_gpio_pin_write(kMuxEnable, 1);
	}


	// Enables the two muxes.
	void Enable()  { nrf_gpio_pin_write(kMuxEnable, 1); }
      

	// Disables the two muxes.
	void Disable() { nrf_gpio_pin_write(kMuxEnable, 0); }

	// Connects output to one of the vibrotactile channels from 0 to 11.
	// One mux connects to the high side of current sensing resistor, and the
	// second mux connects to the low side of current sensing resistor.
	void ConnectChannel(int channel) {
	    static const struct {
		uint8_t i2c_address;  // I2C address of which MAX14661 to configure.
		struct {
		    uint8_t register_address;  // Which register in the MAX14661.
		    uint8_t value;  // Value to write, representing a switch connection.
		} connections[2];
	    }

		kChannelSettings[12] = {
		// Channel 0 (Schematic label: kPwmL1Pin).
		{
		    kMax14661Address2,
		    {{DIR0, 0x02},  // Connects switch AB02 to COMA.
		     {DIR2, 0x01}}  // Connects switch AB01 to COMB.
		},
		// Channel 1 (kPwmR1Pin).
		{
		    kMax14661Address2,
		    {{DIR0, 0x08},  // Connects switch AB04 to COMA.
		     {DIR2, 0x04}}  // Connects switch AB03 to COMB.
		},
		// Channel 2 (kPwmL2Pin).
		{
		    kMax14661Address2,
		    {{DIR0, 0x80},  // Connects switch AB08 to COMA.
		     {DIR2, 0x40}}  // Connects switch AB07 to COMB.
		},
		// Channel 3 (kPwmR2Pin).
		{
		    kMax14661Address2,
		    {{DIR0, 0x20},  // Connects switch AB06 to COMA.
		     {DIR2, 0x10}}  // Connects switch AB05 to COMB.
		},
		// Channel 4 (kPwmL3Pin). <Without noise
		{
		    kMax14661Address1,
		    {{DIR1, 0x02},  // Connects switch AB10 to COMA.
		     {DIR3, 0x01}}  // Connects switch AB09 to COMB.
		},
		// Channel 5 (kPwmR3Pin). <Without noise
		{
		    kMax14661Address1,
		    {{DIR1, 0x08},  // Connects switch AB12 to COMA.
		     {DIR3, 0x04}}  // Connects switch AB11 to COMB.
		},
		// Channel 6 (kPwmL4Pin). <Without noise
		{
		    kMax14661Address1,
		    {{DIR1, 0x80},  // Connects switch AB16 to COMA.
		     {DIR3, 0x40}}  // Connects switch AB15 to COMB.
		},
		// Channel 7 (kPwmR4Pin). <Without noise
		{
		    kMax14661Address1,
		    {{DIR1, 0x20},  // Connects switch AB14 to COMA.
		     {DIR3, 0x10}}  // Connects switch AB13 to COMB.
		},
		// Channel 8 (kPwmL5Pin). <Noisy
		{
		    kMax14661Address1,
		    {{DIR0, 0x02},  // Connects switch AB02 to COMA.
		     {DIR2, 0x01}}  // Connects switch AB01 to COMB.
		},
		// Channel 9 (kPwmR5Pin). <Noisy
		{
		    kMax14661Address1,
		    {{DIR0, 0x08},  // Connects switch AB04 to COMA.
		     {DIR2, 0x04}}  // Connects switch AB03 to COMB.
		},
		// Channel 10 (kPwmL6Pin). <Noisy
		{
		    kMax14661Address1,
		    {{DIR0, 0x80},  // Connects switch AB08 to COMA.
		     {DIR2, 0x40}}  // Connects switch AB07 to COMB.
		},
		// Channel 11 (kPwmR6Pin). <Noisy
		{
		    kMax14661Address1,
		    {{DIR0, 0x20},  // Connects switch AB06 to COMA.
		     {DIR2, 0x10}}  // Connects switch AB05 to COMB.
		},
	    };

	    if (channel < 0 || channel >= 12) {
		return;
	    }

	    // Disconnect all previous switches.
	    DisconnectAllSwitches();

	    const auto& settings = kChannelSettings[channel];
	    NRF_TWIM0->ADDRESS = settings.i2c_address;
	    i2c_write(settings.connections[0].register_address,
		      settings.connections[0].value);
	    i2c_write(settings.connections[1].register_address,
		      settings.connections[1].value);
	}


	// Disconnects all switches inside the two muxes. This means inputs are not
	// connected to outputs.
	void DisconnectAllSwitches() {
	    // Sets registers to zero for mux 1.
	    NRF_TWIM0->ADDRESS = kMax14661Address1;
	    i2c_write(DIR0, 0x00);
	    i2c_write(DIR1, 0x00);
	    i2c_write(DIR2, 0x00);
	    i2c_write(DIR3, 0x00);

	    // Sets registers to zero for mux 2.
	    NRF_TWIM0->ADDRESS = kMax14661Address2;
	    i2c_write(DIR0, 0x00);
	    i2c_write(DIR1, 0x00);
	    i2c_write(DIR2, 0x00);
	    i2c_write(DIR3, 0x00);
	}


    private:
	// Register map constants from the mux datasheet.
	enum {
	    DIR0 = 0x00,
	    DIR1 = 0x01,
	    DIR2 = 0x02,
	    DIR3 = 0x03,
	    SHDW0 = 0x10,
	    SHDW1 = 0x11,
	    SHDW2 = 0x12,
	    SHDW3 = 0x13,
	    CMD_A = 0x14,
	    CMD_B = 0x15
	};
    };

    Max14661 Multiplexer;

}  // namespace audio_tactile

#endif  // MAX14461_HPP_
